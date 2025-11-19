import _thread

import time

# Import the custom stepper module
import stepper

RSTEPPER_PINS = [0, 1, 2, 3]
LSTEPPER_PINS = [7, 6, 5, 4]

STEPS = 12 # 4 = full stepping

ddrive = stepper.DiffDrive(RSTEPPER_PINS, LSTEPPER_PINS)

# Function that runs in a separate thread
def handle_diff_drive():
    ddrive.task()

_thread.start_new_thread(handle_diff_drive, ())


ddrive.set_rpm(-30.0, 50.0)

time.sleep(1)

ddrive.set_rpm(50.0, -30.0)

time.sleep(1)

ddrive.stop()
