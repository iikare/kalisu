import matplotlib.pyplot as plt
import numpy as np

with open("pixels.txt", "r") as f:
    data = np.fromstring(f.read(), sep=" ")

fig, ax = plt.subplots(figsize=(8, 12))

rows = np.arange(len(data))
ax.plot(data, rows, color='black', linewidth=0.7)

ax.set_title("density by row")
ax.set_xlabel("count")
ax.set_ylabel("row")

ax.invert_yaxis()

ax.grid(True, linestyle=':', alpha=0.6)

plt.tight_layout()
plt.show()
