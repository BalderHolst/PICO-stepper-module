import time

# Import the custom stepper module
import stepper

RSTEPPER_PINS = [0, 1, 2, 3]
LSTEPPER_PINS = [7, 6, 5, 4]

STEPS = 12 # 4 = full stepping

rstepper = stepper.Stepper(RSTEPPER_PINS, STEPS)
lstepper = stepper.Stepper(LSTEPPER_PINS, STEPS)

end = time.time() + 5
while time.time() < end:

    # Step the motors
    rstepper.step(True, 0.6)
    lstepper.step(True, 0.6)

    time.sleep_us(500)

# Stop the steppers, releasing their coils
rstepper.stop()
lstepper.stop()
