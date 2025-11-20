#ifndef STEPPER_H
#define STEPPER_H

#include <pico/stdlib.h>

/*
 * Number of pins used for a stepper motor
 */
#define STEPPER_PINS 4

/*
 * Number of step sequences per motor revolution.
 *
 * A step sequence is equivalent to 4 steps using full stepping.
 */
static const int STEPPER_SEQS_PER_REV = 200 / 4;

/*
 * Wrapping value for PWM timer.
 */
static const uint16_t PWM_WRAP = 5000; // Set to get >= 20kHz PWM frequency

/*
 * Maximum PWM level that this library will set to the stepper coils.
 */
static const uint16_t PWM_MAX  = PWM_WRAP * 70/100; // 70% duty cycle

/*
 * Minimum PWM level that this library will set to the stepper coils.
 */
static const uint16_t PWM_MIN  = PWM_WRAP * 20/100; // 20% duty cycle

/*
 * Named constants for different stepping modes.
 *
 * Used when calling `stepper_init`.
 */
enum StepperStepping {
    FULL_STEP    = 1<<2,
    HALF_STEP    = 1<<3,
    QUARTER_STEP = 1<<4,
    EIGHTH_STEP  = 1<<5,
};

/*
 * PWM sequence structure.
 *
 * Holds the PWM levels for each coil for each step in the sequence.
 * This items are dynamically allocated and must be freed when no longer needed.
 */
typedef struct {
    float * items;
    size_t length;
} PWMSequence;


/*
 * Stepper motor structure.
 */
// TODO: Make pins part of this struct
typedef struct {
    int * pins;           // Pins connected to the stepper motor
    PWMSequence sequence; // PWM sequence for the stepper motor
    int t;                // The current step
} Stepper;

/*
 * Initialize a stepper motor with given pins and steps per sequence.
 *
 * Refer to the `StepperStepping` enum for different stepping modes.
 *
 * This method allocates memory for the PWM sequence internally using
 * the `malloc` from the PICO SDK. See `stepper_deinit` for freeing the memory.
 *
 * See also `stepper_init_with_seq` and `stepper_generate_seq` for more control
 * over memory allocation.
 */
void stepper_init(Stepper * stepper, int pins[STEPPER_PINS], int steps_pr_seq);

/*
 * Deinitialize a stepper motor, stopping it and freeing allocated memory.
 */
void stepper_deinit(Stepper * stepper);

/*
 * Step the stepper motor a single step in the steppering sequence.
 *
 * `direction` is true for forward, false for backward.
 * `level` is the PWM level to set for the step (0 to PWM_MAX).
 */
void stepper_step(Stepper* stepper, bool direction, uint16_t level);

/*
 * Set the PWM levels for the stepper motor pins.
 *
 * `state` is an array of PWM levels with a maximum of `PWM_MAX`.
 * To get 20% duty cycle, set level to `PWM_WRAP * 20/100`.
 */
void stepper_set_pins(Stepper * stepper, uint16_t state[STEPPER_PINS]);

/*
 * Stop the stepper motor by setting all pins to 0 PWM level.
 *
 * This releases the motor and allows it to spin freely.
 */
void stepper_stop(Stepper* stepper);

/*
 * Generate a stepping sequence with given number of steps into the provided table.
 *
 * This function *does no allocations*. The caller must provide a float array of
 * size `steps * STEPPER_PINS`.
 *
 * The caller is responsible for freeing the allocated memory.
 */
PWMSequence stepper_generate_seq(uint steps, float * table);

/*
 * Initialize a stepper motor with given pins and a pre-generated PWM sequence.
 *
 * This method does not allocate memory.
 */
void stepper_init_with_seq(Stepper * stepper, int pins[STEPPER_PINS], PWMSequence seq);

#endif // STEPPER_H
