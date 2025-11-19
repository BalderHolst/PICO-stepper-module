# PICO Stepper Module
This repository contains a [micropython](https://micropython.org/) module for running stepper motors with the [Raspberry Pi PICO](https://www.raspberrypi.com/documentation/microcontrollers/pico-series.html) implemented in C.

Refer to the [`stepper.pyi`](micropython_module/stepper.pyi) stub file for an API reference.

```text
.
├── build.py            Script for building micropython with the `stepper` module
├── examples            Examples of using the micropython module
├── flake.nix           Nix flake for a reproducible dev enviornment
├── micropython_module  Python bindings for the stepper library
└── stepper             Pure C stepper library (Link with PICO C SDK)
```

## Examples

### Running a Stepper Motor
```python
import time
import stepper # Custom C extension

STEPS = 12 # 4 = full stepping

# Initialize the stepper motor
motor = stepper.Stepper([0, 1, 2, 3], STEPS)

# Loop for 5 seconds
end = time.time() + 5
while time.time() < end:

    # Step the motor forwards (`True`) with 20% (`0.2`) power
    motor.step(True, 0.2)

    # Sleep before next step, this delay sets the motor speed
    time.sleep_us(1000)

# Stop the stepper, releasing its coils
motor.stop()
```

### Running a Differential Drive
```python
import _thread
import time
import stepper # Custom C extension

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
    ddrive.set_trans_rot(50.0, rot)
    time.sleep_ms(10)

time.sleep(1)

# Stop the motors, releasing their coils
ddrive.stop()
```

**IMPORTANT**: Any method of `DiffDrive` beginning with `set_` *will hang* if the `task_loop` is not running.

## Building Micropython with Extension
Begin by cloning the repository

```bash
git clone https://github.com/BalderHolst/PICO-stepper-module
cd PICO-stepper-module
```

The `build.py` script provides an automated way of compiling Micropython with the `stepper` module.

All options can be displayed by providing the `--help` option:
```bash
./build.py --help
```

***Output:***
```text
usage: build.py [-h] [--clean] [--rebuild] [--flash]

Script for building Micropython with the `stepper` module.

options:
  -h, --help  show this help message and exit
  --clean     Clean build artifacts before building
  --rebuild   Clean and rebuild Micropython
  --flash     Flash the built firmware to the PICO after building
```

To build the micropython firmware, simply run the script:
```
./build.py
```

This will download and compile micropython with the `stepper` module.

When finished, a "build" directory should be created.

To flash the firmware to a PICO do the following:
1. Disconnect the PICO if it is on
2. Hold down the BOOTSEL button on the PICO
3. While holding, connect the PICO to the computer through USB
4. You can now release the BOOTSEL button
5. Run: `./build.py --flash` (this will also rebuild the firmware if needed)


## Testing it out!
Run an example using the `mpremote` tool:

```bash
mpremote run examples/stepper.py
```

You can also use it to upload an example as `main.py` to run it on startup:
```bash
mpremote cp examples/diff_drive.py :/main.py
```

## Contributions
Pull requests are very much welcome!
