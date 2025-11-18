#include <stdlib.h>
#include <pico/stdlib.h>
#include <hardware/pwm.h>
#include <math.h>

#include "stepper.h"
#include "utils.h"

const float PI = M_PI;

const float COIL_PHASES[] = {
    0 * PI / 2,
    1 * PI / 2,
    2 * PI / 2,
    3 * PI / 2,
};

static PWMSequence generate_PWM_sequence(uint steps) {
    float * table = malloc(sizeof(float) * steps * STEPPER_PINS);

    if (!table) PANIC("Failed to allocate memory for PWM sequence");

    for (uint step = 0; step < steps; step++) {
        float t = 2 * PI * (float)step / (float)steps;
        for (int coil = 0; coil < STEPPER_PINS; coil++) {
            size_t idx = step * STEPPER_PINS + coil;

            // Approximate sine wave
            float y = sin(t + COIL_PHASES[coil]);

            // Only positive half wave
            y = MAX(y, 0);

            table[idx] = y;
        }
    }


    PWMSequence seq = {0};
    seq.items = table;
    seq.length = steps;

    return seq;
}

static void state_to_levels(float state[STEPPER_PINS], uint16_t levels[STEPPER_PINS], uint16_t pwm) {
    for (int i = 0; i < STEPPER_PINS; i++) {
        levels[i] = (uint16_t)(state[i] * pwm);
    }
}

Stepper stepper_init(int pins[STEPPER_PINS], int steps_pr_seq) {
    Stepper stepper = {0};

    stepper.pins = pins;
    stepper.sequence = generate_PWM_sequence(steps_pr_seq);
    stepper.t = 0;

    for (int i = 0; i < STEPPER_PINS; i++) {
        uint pin = pins[i];

        gpio_set_function(pin, GPIO_FUNC_PWM);
        uint slice_num = pwm_gpio_to_slice_num(pin);

        // 16 bit resolution
        pwm_set_wrap(slice_num, PWM_WRAP);
        pwm_set_enabled(slice_num, true);
        pwm_set_clkdiv(slice_num, 1.0f);

        uint16_t levels[STEPPER_PINS] = {0};
        state_to_levels(stepper.sequence.items, levels, PWM_MIN);

        stepper_set_pins(&stepper, levels);
    }

    return stepper;
}

void stepper_set_pins(Stepper * stepper, uint16_t state[STEPPER_PINS]) {
    for (int i = 0; i < STEPPER_PINS; i++) {

        // Make sure that level is within bounds
        uint16_t level = state[i] > PWM_MAX ? PWM_MAX : state[i];

        pwm_set_gpio_level(stepper->pins[i], level);
    }
}

void stepper_step(Stepper* stepper, bool direction, uint16_t level) {
    // Step the stepper in the given direction
    stepper->t += direction ? 1 : -1;

    // Wrap around if negative
    if (stepper->t < 0) stepper->t += stepper->sequence.length;

    // Wrap around if exceeding length
    stepper->t = stepper->t % stepper->sequence.length;

    float * state = &stepper->sequence.items[stepper->t * STEPPER_PINS];

    uint16_t levels[STEPPER_PINS] = {0};
    state_to_levels(state, levels, level);

    stepper_set_pins(stepper, levels);
}

void stepper_stop(Stepper* stepper) {
    uint16_t levels[STEPPER_PINS] = {0, 0, 0, 0};
    stepper_set_pins(stepper, levels);
}
