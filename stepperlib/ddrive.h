#ifndef DIFF_DRIVE_H
#define DIFF_DRIVE_H

#include "stepper.h"
#include "interp.h"

/*
 * A good value for steps per sequence for diff drive motors.
 */
static const uint DEFAULT_DDRIVE_STEPS_PR_SEQ = 128;

/*
 * Rpm at which the stepper motor reaches maximum PWM level.
 */
static const float DDRIVE_MAX_PWM_SPEED = 300.0f;

/*
 * Rpm at which the stepper motor reaches minimum PWM level.
 */
static const float DDRIVE_MIN_PWM_SPEED =   0.0f;

/*
 * Command types for differential drive.
 */
typedef enum {
    DDRIVE_LEFT_RIGHT,
    DDRIVE_TRANS_ROTATE,
    DDRIVE_STOP,
    DDRIVE_TRAPEZOID,
} DiffDriveCmdType;

/*
 * Differential drive command structure.
 */
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

/*
 * Predefined stop command.
 */
static const DiffDriveCmd DDRIVE_CMD_STOP = {
    .type = DDRIVE_STOP,
};

/*
 * Differential drive structure.
 */
typedef struct {

    // Right and left stepper motors
    Stepper rstepper;
    Stepper lstepper;

    // Target RPMs for the motors
    float rrpm;
    float lrpm;

    // Next command handling. See `ddrive_task`.
    DiffDriveCmd next_cmd;
    bool new_cmd_available;

    // For trapezoidal velocity profile
    Interp rinterp;
    Interp linterp;
    bool interp_active;

} DiffDrive;

/*
 * Initialize a differential drive with given pins and steps per sequence.
 *
 * This method allocates memory for the PWM sequences internally using
 * the `malloc` from the PICO SDK. See `ddrive_deinit` for freeing the memory.
 *
 * See also `ddrive_init_with_seq` for more control over memory allocation.
 */
void ddrive_init(DiffDrive * ddrive, int * lpins, int * rpins, size_t steps_pr_seq);

/*
 * Initialize a differential drive with given pins and PWM sequence.
 *
 * Use the `stepper_generate_seq` function from `stepper.h` to create a PWM sequence.
 */
void ddrive_init_with_seq(DiffDrive * ddrive, int * rpins, int * lpins, PWMSequence seq);

/*
 * Update the internal state of the differential drive. This function should be
 * called periodically in a dedicated task or main loop. It handles motor control
 * and command processing. All methods that send commands to the differential drive
 * will not work unless this function is called regularly.
 */
void ddrive_task(DiffDrive * ddrive);

/*
 * Execute a differential drive command. This function is called internally by
 * `ddrive_task` when a new command is available.
 */
void ddrive_handle_command(DiffDrive * ddrive, DiffDriveCmd * cmd);

/*
 * Stop the differential drive motors, allowing them to coast.
 */
void ddrive_stop(DiffDrive * ddrive);

/*
 * Set the target RPMs for the right and left motors.
 */
void ddrive_rpm(DiffDrive * ddrive, float rrpm, float lrpm);

/*
 * Set the target translational and rotational velocities.
 *
 * If `rot` is zero, `trans` is the rpm for both motors.
 * `rot` is the difference between motor rpms.
 */
void ddrive_trans_rot(DiffDrive * ddrive, float trans, float rot);

// TODO: These commands don't always work as expected.
//       I think it has to do with the interpolators.
bool * ddrive_trap_rpm(DiffDrive * ddrive, float rtarget, float ltarget, float time);
bool * ddrive_trap_trans_rot(DiffDrive * ddrive, float trans, float rot, float time);

#endif // DIFF_DRIVE_H
