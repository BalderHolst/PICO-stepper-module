import _thread

import time

# Import the custom stepper module
import stepper

# GPIO pins of the stepper motors
RSTEPPER_PINS = [0, 1, 2, 3]
LSTEPPER_PINS = [7, 6, 5, 4]

STEPS = 128 # 4 = full stepping

ddrive = stepper.DiffDrive(RSTEPPER_PINS, LSTEPPER_PINS, STEPS)

# The stepping the differential drive is done in a separate thread
def handle_diff_drive():
    # Run the differential drive task loop
    ddrive.task_loop()

_thread.start_new_thread(handle_diff_drive, ())

# You can now set the motor speeds directly
ddrive.set_rpm(50, -30)
time.sleep(1)

# Or set translational and rotational speeds
for rot in range(-200, 200):
    ddrive.set_trans_rot(50.0, float(rot))
    time.sleep_ms(10)

time.sleep(1)

# Stop the motors, releasing their coils
ddrive.stop()
