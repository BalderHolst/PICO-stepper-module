import time

# Import the custom stepper module
import stepper

# GPIO pins of the stepper motor
STEPPER_PINS = [0, 1, 2, 3]

STEPS = 12 # 4 = full stepping

# Initialize the stepper motor
motor = stepper.Stepper(STEPPER_PINS, STEPS)

# Loop for 5 seconds
end = time.time() + 5
while time.time() < end:

    # Step the motors
    motor.step(True, 0.2)

    time.sleep_us(1000)

# Stop the stepper, releasing its coils
motor.stop()
