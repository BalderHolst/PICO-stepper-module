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
