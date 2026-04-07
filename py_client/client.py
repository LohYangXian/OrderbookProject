import socket
import time
import random
import csv

HOST = 'localhost'
PORT = 8000
NUM_ORDERS = 10000
CLIENT_ID = 1
SYM = "BTC/USDT"

order_ids = []
rtts = []
type_rtts = {"N": [], "M": [], "C": []}
next_order_id = 1
ts = 0

def random_order():
    """Return (cmd, fix_message_string) for a randomly chosen order action.

    FIX-like format: ts=<ts>|client=<id>|cmd=<N|M|C>|id=<oid>|sym=<sym>|side=<B|S>|px=<price>|qty=<qty>
    Cancel messages omit sym/side/px/qty fields.
    """
    global next_order_id, ts
    ts += 1
    cmd = random.choices(["N", "M", "C"], weights=[0.7, 0.2, 0.1])[0]
    if cmd == "N" or not order_ids:
        oid = next_order_id
        next_order_id += 1
        order_ids.append(oid)
        px = random.randint(90000, 110000)
        qty = random.randint(1, 10)
        side = random.choice(["B", "S"])
        msg = f"ts={ts}|client={CLIENT_ID}|cmd=N|id={oid}|sym={SYM}|side={side}|px={px}|qty={qty}"
        return "N", msg
    elif cmd == "M":
        oid = random.choice(order_ids)
        px = random.randint(90000, 110000)
        qty = random.randint(1, 10)
        side = random.choice(["B", "S"])
        msg = f"ts={ts}|client={CLIENT_ID}|cmd=M|id={oid}|sym={SYM}|side={side}|px={px}|qty={qty}"
        return "M", msg
    else:  # C - cancel
        oid = random.choice(order_ids)
        order_ids.remove(oid)
        msg = f"ts={ts}|client={CLIENT_ID}|cmd=C|id={oid}"
        return "C", msg

for _ in range(NUM_ORDERS):
    cmd, msg = random_order()
    data = msg.encode()
    t0 = time.time_ns()
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        s.sendall(data)
        response = s.recv(4096)
    t1 = time.time_ns()
    rtt_us = (t1 - t0) // 1000
    rtts.append(rtt_us)
    type_rtts[cmd].append(rtt_us)

# # Save RTTs to CSV
# with open("rtts.csv", "w", newline="") as f:
#     writer = csv.writer(f)
#     writer.writerow(["RTT_us"])
#     for rtt in rtts:
#         writer.writerow([rtt])

# for typ in type_rtts:
#     with open(f"rtts_{typ}.csv", "w", newline="") as f:
#         writer = csv.writer(f)
#         writer.writerow([f"RTT_us_{typ}"])
#         for rtt in type_rtts[typ]:
#             writer.writerow([rtt])

# Print metrics
import numpy as np
def print_stats(name, data):
    arr = np.array(data)
    print(f"{name}: count={len(arr)}, mean={arr.mean():.2f}us, p95={np.percentile(arr,95):.2f}us, p99={np.percentile(arr,99):.2f}us, max={arr.max():.2f}us")

print_stats("ALL", rtts)
for typ in type_rtts:
    print_stats(typ, type_rtts[typ])

import matplotlib.pyplot as plt

# Histogram for all RTTs
plt.figure(figsize=(10,6))
plt.hist(rtts, bins=100, alpha=0.7)
plt.title("RTT Histogram (All Orders)")
plt.xlabel("RTT (us)")
plt.ylabel("Count")
plt.show()

# Histogram for each type
for typ in type_rtts:
    plt.figure(figsize=(10,6))
    plt.hist(type_rtts[typ], bins=100, alpha=0.7)
    plt.title(f"RTT Histogram ({typ} Orders)")
    plt.xlabel("RTT (us)")
    plt.ylabel("Count")
    plt.show()

# # Boxplot for all types
# plt.figure(figsize=(8,6))
# plt.boxplot([type_rtts[typ] for typ in type_rtts], labels=list(type_rtts.keys()))
# plt.title("RTT Boxplot by Order Type")
# plt.ylabel("RTT (us)")
# plt.show()

# # CDF for all types
# for typ in type_rtts:
#     data = np.sort(type_rtts[typ])
#     cdf = np.arange(len(data)) / float(len(data))
#     plt.plot(data, cdf, label=typ)
# plt.title("RTT CDF by Order Type")
# plt.xlabel("RTT (us)")
# plt.ylabel("CDF")
# plt.legend()
# plt.show()