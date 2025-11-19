#include <hardware/gpio.h>
#include <pico/time.h>
#include <math.h>
#include <pico/stdlib.h>
#include <stdlib.h>

#include "ddrive.h"
#include "interp.h"
#include "stepper.h"

#define CLAMP(x, lower, upper) ((x) < (lower) ? (lower) : ((x) > (upper) ? (upper) : (x)))

void ddrive_init(DiffDrive * ddrive, int * lpins, int * rpins) {
    float * buf = malloc(sizeof(float) * DDRIVE_STEPS_PR_SEQ * STEPPER_PINS);
    PWMSequence seq = stepper_generate_seq(DDRIVE_STEPS_PR_SEQ, buf);
    ddrive_init_with_seq(ddrive, lpins, rpins, seq);
}

void ddrive_init_with_seq(DiffDrive * ddrive, int * lpins, int * rpins, PWMSequence seq) {
    stepper_init_with_seq(&ddrive->lstepper ,lpins, DDRIVE_STEPS_PR_SEQ, seq);
    stepper_init_with_seq(&ddrive->rstepper, rpins, DDRIVE_STEPS_PR_SEQ, seq);

    ddrive->lrpm     = 0;
    ddrive->rrpm     = 0;

    ddrive->linterp   = (Interp){0};
    ddrive->rinterp   = (Interp){0};

    ddrive->new_cmd_available = false;
    ddrive->next_cmd = DDRIVE_CMD_STOP;
}

static void stop_interpolators(DiffDrive * ddrive) {
    ddrive->rinterp.running = false;
    ddrive->linterp.running = false;
}

static void trans_rot_to_rpm(float trans, float rot, float * lrpm, float * rrpm) {
    *rrpm = 0;
    *lrpm = 0;

    float abs_rot = fabs(rot);
    float dir_rot = (rot >= 0) ? 1 : -1;

    *rrpm -= abs_rot / 2 * dir_rot;
    *lrpm += abs_rot / 2 * dir_rot;

    *rrpm += trans;
    *lrpm += trans;
}

void ddrive_handle_command(DiffDrive * ddrive, DiffDriveCmd * cmd) {
    switch (cmd->type) {
        case DDRIVE_LEFT_RIGHT:
            stop_interpolators(ddrive);
            ddrive->rrpm = cmd->right;
            ddrive->lrpm = cmd->left;
            break;
        case DDRIVE_TRANS_ROTATE:
            stop_interpolators(ddrive);
            trans_rot_to_rpm(cmd->trans, cmd->rot, &ddrive->lrpm, &ddrive->rrpm);
            break;
        case DDRIVE_TRAPEZOID: {
            uint64_t time_us = cmd->time * 1e6;
            interp_start(&ddrive->rinterp, ddrive->rrpm, cmd->rtarget, time_us);
            interp_start(&ddrive->linterp, ddrive->lrpm, cmd->ltarget, time_us);
        } break;
        case DDRIVE_STOP:
            stop_interpolators(ddrive);
            ddrive->rrpm = 0;
            ddrive->lrpm = 0;
            stepper_stop(&ddrive->rstepper);
            stepper_stop(&ddrive->lstepper);
            break;
    }
}

const uint64_t MAX_STEP_US  = 100;
const uint64_t ZERO_STEP_US = 100;

void ddrive_task(DiffDrive * ddrive) {

    // Handle new command if available
    if (ddrive->new_cmd_available) {
        ddrive_handle_command(ddrive, &ddrive->next_cmd);
        ddrive->new_cmd_available = false;
    }

    static float fast_rpm, slow_rpm, us_pr_step, ratio;
    static bool fast_dir, slow_dir;

    static Stepper * fast_stepper;
    static Stepper * slow_stepper;

    // Update RPMs if interpolating
    if (ddrive->rinterp.running) ddrive->rrpm = interp_value(&ddrive->rinterp);
    if (ddrive->linterp.running) ddrive->lrpm = interp_value(&ddrive->linterp);

    // Disable steppers if RPM it should not move
    if (ddrive->rrpm == 0.0) stepper_stop(&ddrive->rstepper);
    if (ddrive->lrpm == 0.0) stepper_stop(&ddrive->lstepper);

    bool rforward = ddrive->rrpm >= 0;
    bool lforward = ddrive->lrpm >= 0;

    float abs_rrpm = fabs(ddrive->rrpm);
    float abs_lrpm = fabs(ddrive->lrpm);

    // Determine which stepper is faster for diff drive
    if (abs_rrpm > abs_lrpm) {
        fast_stepper = &ddrive->rstepper;
        fast_dir = rforward;
        fast_rpm = abs_rrpm;

        slow_stepper = &ddrive->lstepper;
        slow_rpm = abs_lrpm;
        slow_dir = lforward;
    } else {
        fast_stepper = &ddrive->lstepper;
        fast_rpm = abs_lrpm;
        fast_dir = lforward;

        slow_stepper = &ddrive->rstepper;
        slow_rpm = abs_rrpm;
        slow_dir = rforward;
    }

    if (fast_rpm == 0) {
        interp_tick(&ddrive->rinterp, ZERO_STEP_US);
        interp_tick(&ddrive->linterp, ZERO_STEP_US);
        sleep_us(ZERO_STEP_US);
        return;
    };

    us_pr_step = MIN((60 * 1e6) / (DDRIVE_STEPS_PR_REV * fast_rpm), MAX_STEP_US);
    ratio      = slow_rpm / fast_rpm;

    // Accumulates the fractional steps for the slow stepper
    static float step_acc = 0;

    // Update interpolators
    bool rdone = interp_tick(&ddrive->rinterp, DDRIVE_STEPS_PR_SEQ * us_pr_step);
    bool ldone = interp_tick(&ddrive->linterp, DDRIVE_STEPS_PR_SEQ * us_pr_step);
    ddrive->interp_active = !(rdone && ldone);


    float    slow_t     = (fabs(slow_rpm) - DDRIVE_MIN_PWM_SPEED) / (DDRIVE_MAX_PWM_SPEED - DDRIVE_MIN_PWM_SPEED);
    uint16_t slow_level = (PWM_MAX - PWM_MIN) * slow_t + PWM_MIN;

    float    fast_t     = (fabs(fast_rpm) - DDRIVE_MIN_PWM_SPEED) / (DDRIVE_MAX_PWM_SPEED - DDRIVE_MIN_PWM_SPEED);
    uint16_t fast_level = (PWM_MAX - PWM_MIN) * fast_t + PWM_MIN;

    slow_level = CLAMP(slow_level, PWM_MIN, PWM_MAX);
    fast_level = CLAMP(fast_level, PWM_MIN, PWM_MAX);

    for (int i = 0; i < DDRIVE_STEPS_PR_SEQ; i++) {
        // Step the fast stepper every iteration
        stepper_step(fast_stepper, fast_dir, fast_level);

        // Accumulate fractional steps for the slow stepper
        step_acc += ratio;

        // Step the slow stepper when enough fractional steps have accumulated
        if (step_acc >= 1.0f) {
            step_acc -= 1.0f;
            stepper_step(slow_stepper, slow_dir, slow_level);
        }

        sleep_us(us_pr_step);
    }

}

// ==================== COMMANDS ====================
static void send_cmd(DiffDrive * ddrive, DiffDriveCmd cmd) {
    while (ddrive->new_cmd_available) {
        tight_loop_contents();
    }
    ddrive->next_cmd = cmd;
    ddrive->new_cmd_available = true;
}

void ddrive_stop(DiffDrive * ddrive) {
    send_cmd(ddrive, DDRIVE_CMD_STOP);
}

void ddrive_rpm(DiffDrive * ddrive, float rrpm, float lrpm) {
    DiffDriveCmd cmd = {
        .type  = DDRIVE_LEFT_RIGHT,
        .right = rrpm,
        .left  = lrpm,
    };
    send_cmd(ddrive, cmd);
}

void ddrive_trans_rot(DiffDrive * ddrive, float trans, float rot) {
    DiffDriveCmd cmd = {
        .type  = DDRIVE_TRANS_ROTATE,
        .trans = trans,
        .rot   = rot,
    };
    send_cmd(ddrive, cmd);
}

bool * ddrive_trap_rpm(DiffDrive * ddrive, float ltarget, float rtarget, float time) {
    DiffDriveCmd cmd = {
        .type  = DDRIVE_TRAPEZOID,
        .ltarget = ltarget,
        .rtarget = rtarget,
        .time    = time,
    };
    send_cmd(ddrive, cmd);
    return &ddrive->interp_active;
}

bool * ddrive_trap_trans_rot(DiffDrive * ddrive, float trans_target, float rot_target, float time) {
    DiffDriveCmd cmd = {
        .type  = DDRIVE_TRAPEZOID,
        .ltarget = 0,
        .rtarget = 0,
        .time    = time,
    };
    trans_rot_to_rpm(trans_target, rot_target, &cmd.ltarget, &cmd.rtarget);
    send_cmd(ddrive, cmd);
    return &ddrive->interp_active;
}
