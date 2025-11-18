#ifndef DIFF_DRIVE_H
#define DIFF_DRIVE_H

#include <pico/util/queue.h>

#include "stepper.h"
#include "interp.h"

static const uint DDRIVE_STEPS_PR_SEQ = 128;
static const uint DDRIVE_STEPS_PR_REV = DDRIVE_STEPS_PR_SEQ * STEPPER_SEQS_PER_RV;

static const uint DDRIVE_QUEUE_SIZE = 10;

static const float DDRIVE_MAX_PWM_SPEED = 400.0f;
static const float DDRIVE_MIN_PWM_SPEED =   0.0f;

typedef enum {
    DDRIVE_LEFT_RIGHT,
    DDRIVE_TRANS_ROTATE,
    DDRIVE_STOP,
    DDRIVE_TRAPEZOID,
} DiffDriveCmdType;

typedef struct {
    DiffDriveCmdType type;
    union {
        struct { float left; float right;  };
        struct { float trans; float rot;  };
        struct {
            float ltarget;
            float rtarget;
            float time;
        };
    };
} DiffDriveCmd;

static const DiffDriveCmd DDRIVE_CMD_STOP = {
    .type = DDRIVE_STOP,
};

typedef struct {
    Stepper rstepper;
    Stepper lstepper;
    float rrpm;
    float lrpm;
    Stepper * fast_stepper;
    Stepper * slow_stepper;
    float fast_rpm;
    float slow_rpm;
    uint us_pr_step;
    float ratio;

    // Command queue
    queue_t cmd_queue;

    // For trapezoidal velocity profile
    Interp rinterp;
    Interp linterp;
    bool interp_active;

} DiffDrive;

DiffDrive ddrive_init(int * rpins, int * lpins);

void ddrive_handle_command(DiffDrive * ddrive, DiffDriveCmd * cmd);
void ddrive_task(DiffDrive * ddrive);

void ddrive_stop(DiffDrive * ddrive);
void ddrive_rpm(DiffDrive * ddrive, float rrpm, float lrpm);
void ddrive_trans_rot(DiffDrive * ddrive, float trans, float rot);
bool * ddrive_trap_rpm(DiffDrive * ddrive, float rtarget, float ltarget, float time);
bool * ddrive_trap_trans_rot(DiffDrive * ddrive, float trans, float rot, float time);

#endif // DIFF_DRIVE_H
