import matplotlib.pyplot as plt

times = []
rtts = []
with open('./rtt.txt', 'r') as f:
    for line in f:
        if line.strip() == "":
            continue
        t, rtt = line.strip().split()
        times.append(float(t))
        rtts.append(float(rtt))

plt.figure(figsize=(10, 5))
plt.plot(times, rtts, marker='o', linestyle='-', markersize=2)
plt.xlabel('Time (s)')
plt.ylabel('RTT (ms)')
plt.title('RTT vs Time')
plt.grid(True)
plt.tight_layout()
plt.show()
