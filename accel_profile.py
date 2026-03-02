import numpy as np
import matplotlib.pyplot as plt

total_steps   = 20000
accel_steps   = total_steps >> 2
f_start       = 200
f_max         = 4000
timer_clock   = 2_000_000  # Hz

curve_freq = np.zeros(total_steps, dtype=int)

def fp_mul(a: int, b: int):
    return (a * b) >> 16

def fp_div(a: int, b: int):
    if b == 0:
        return 0
    return (a << 16) // b

def smoothstep_qubic(x: int):
    # x in Q16
    return fp_mul(x, (3<<16) - fp_mul(2<<16, x))

# -------- ACCELERATION --------
for i in range(accel_steps):
    s = (i * (1<<16)) // accel_steps
    scale = smoothstep_qubic(s)
    freq = f_start + (fp_mul((f_max - f_start)<<16, scale) >> 16)
    curve_freq[i] = timer_clock // freq

# -------- CONSTANT SPEED --------
for i in range(accel_steps, total_steps - accel_steps):
    curve_freq[i] = timer_clock // f_max

# -------- DECELERATION --------
for i in range(total_steps - accel_steps, total_steps):
    s = ((total_steps - i) * (1<<16)) // accel_steps
    scale = smoothstep_qubic(s)
    freq = f_start +( fp_mul((f_max - f_start)<<16, scale) >> 16)
    curve_freq[i] = timer_clock // freq

plt.plot(curve_freq)
plt.title("S-Curve Frequency Profile")
plt.ylabel("Timer reload value")
plt.xlabel("Step index")
plt.tight_layout()
plt.show()