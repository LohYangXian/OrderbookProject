import socket
import time
import json
import random
import csv

HOST = 'localhost'
PORT = 8000
NUM_ORDERS = 10000

order_ids = []
rtts = []
type_rtts = {"NEW": [], "MODIFY": [], "CANCEL": []}
next_order_id = 1

def random_order():
    global next_order_id
    order_type = random.choices(["NEW", "MODIFY", "CANCEL"], weights=[0.7, 0.2, 0.1])[0]
    if order_type == "NEW" or not order_ids:
        order = {
            "Type": "NEW",
            "OrderId": next_order_id,
            "Pair": "BTC/USDT",
            "Price": str(random.randint(90000, 110000)),
            "Quantity": str(random.randint(1, 10)),
            "Side": random.choice(["BUY", "SELL"])
        }
        order_ids.append(next_order_id)
        next_order_id += 1
    elif order_type == "MODIFY":
        oid = random.choice(order_ids)
        order = {
            "Type": "MODIFY",
            "OrderId": oid,
            "Pair": "BTC/USDT",
            "Price": str(random.randint(90000, 110000)),
            "Quantity": str(random.randint(1, 10)),
            "Side": random.choice(["BUY", "SELL"])
        }
    else:  # CANCEL
        oid = random.choice(order_ids)
        order = {
            "Type": "CANCEL",
            "OrderId": oid
        }
        order_ids.remove(oid)
    return order_type, order

for _ in range(NUM_ORDERS):
    order_type, order = random_order()
    msg = json.dumps(order).encode()
    t0 = time.time_ns()
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        s.sendall(msg)
        response = s.recv(4096)
    t1 = time.time_ns()
    rtt_us = (t1 - t0) // 1000
    rtts.append(rtt_us)
    type_rtts[order_type].append(rtt_us)

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