#ifndef STEPPER_H
#define STEPPER_H

#include <pico/stdlib.h>

#define STEPPER_PINS 4

static const int STEPPER_SEQS_PER_RV = 200 / 4;

static const uint16_t PWM_WRAP = 5000;              // Set to get >= 20kHz PWM frequency
static const uint16_t PWM_MAX  = PWM_WRAP * 70/100; // 70% duty cycle
static const uint16_t PWM_MIN  = PWM_WRAP * 20/100; // 20% duty cycle

enum StepperStepping {
    FULL_STEP    = 1<<2,
    HALF_STEP    = 1<<3,
    QUARTER_STEP = 1<<4,
    EIGHTH_STEP  = 1<<5,
};

typedef struct {
    float * items;
    size_t length;
} PWMSequence;

typedef struct {
    int * pins;           // Pins connected to the stepper motor
    PWMSequence sequence; // PWM sequence for the stepper motor
    int t;                // The current step
} Stepper;

Stepper stepper_init(int pins[STEPPER_PINS], int steps_pr_seq);
Stepper stepper_init_with_buf(int pins[STEPPER_PINS], int steps_pr_seq, float * buf);
void stepper_deinit(Stepper * stepper);
void stepper_set_pins(Stepper * stepper, uint16_t state[STEPPER_PINS]);
void stepper_step(Stepper* stepper, bool direction, uint16_t level);
void stepper_stop(Stepper* stepper);


#endif // STEPPER_H
