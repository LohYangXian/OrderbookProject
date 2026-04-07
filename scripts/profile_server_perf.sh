#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   ./scripts/profile_server_perf.sh [duration_seconds] [output_dir] [client_command] [num_threads]
# Examples:
#   ./scripts/profile_server_perf.sh
#   ./scripts/profile_server_perf.sh 20 results/profile_run_01
#   ./scripts/profile_server_perf.sh 20 results/profile_run_01 "python3 py_client/client.py"
#   ./scripts/profile_server_perf.sh 20 results/profile_run_01 "" 4

DURATION="${1:-15}"
OUTDIR="${2:-results/profile}"
CLIENT_CMD="${3:-}"
NUM_THREADS="${4:-1}"

# Convenience: allow shorthand `... <duration> <outdir> <num_threads>`.
# Example: ./scripts/profile_server_perf.sh 20 results/run_01 4
if [[ -n "${CLIENT_CMD}" && -z "${4:-}" && "${CLIENT_CMD}" =~ ^[0-9]+$ ]]; then
  NUM_THREADS="${CLIENT_CMD}"
  CLIENT_CMD=""
fi

if ! [[ "${NUM_THREADS}" =~ ^[0-9]+$ ]] || [[ "${NUM_THREADS}" -lt 1 ]]; then
  echo "Error: num_threads must be a positive integer. Got: ${NUM_THREADS}"
  echo "Usage: ./scripts/profile_server_perf.sh [duration_seconds] [output_dir] [client_command] [num_threads]"
  echo "Examples:"
  echo "  ./scripts/profile_server_perf.sh 20 results/profile_run_01"
  echo "  ./scripts/profile_server_perf.sh 20 results/profile_run_01 \"\" 4"
  echo "  ./scripts/profile_server_perf.sh 20 results/profile_run_01 \"python3 py_client/client.py\""
  exit 1
fi

resolve_perf_bin() {
  # Try perf from PATH first.
  if command -v perf >/dev/null 2>&1; then
    if perf version >/dev/null 2>&1; then
      command -v perf
      return 0
    fi
  fi

  # WSL often lacks an exact kernel-matched linux-tools package.
  # Fall back to any installed perf binary under linux-tools.
  local candidate
  candidate="$(ls -1 /usr/lib/linux-tools/*/perf 2>/dev/null | sort -V | tail -n 1 || true)"
  if [[ -n "${candidate}" && -x "${candidate}" ]]; then
    echo "${candidate}"
    return 0
  fi

  return 1
}

if ! PERF_BIN="$(resolve_perf_bin)"; then
  echo "Error: perf is not installed in this environment."
  echo "Install on Ubuntu/WSL (do NOT use linux-tools-\$(uname -r) on WSL):"
  echo "  sudo apt-get update"
  echo "  sudo apt-get install -y linux-tools-common linux-tools-generic linux-cloud-tools-generic"
  exit 1
fi

echo "Using perf binary: ${PERF_BIN}"

mkdir -p "${OUTDIR}"

echo "[1/5] Building server with profiling-friendly flags..."
bazel build -c opt --strip=never --copt=-g --copt=-fno-omit-frame-pointer //src:main_server

echo "[2/5] Starting server..."
./bazel-bin/src/main_server >"${OUTDIR}/server.log" 2>&1 &
SERVER_PID=$!

cleanup() {
  if kill -0 "${SERVER_PID}" >/dev/null 2>&1; then
    kill "${SERVER_PID}" >/dev/null 2>&1 || true
    wait "${SERVER_PID}" 2>/dev/null || true
  fi
}
trap cleanup EXIT

sleep 1

if ! kill -0 "${SERVER_PID}" >/dev/null 2>&1; then
  echo "Error: server failed to start. Check ${OUTDIR}/server.log"
  exit 1
fi

echo "Server PID: ${SERVER_PID}"

CLIENT_PID=""
if [[ -n "${CLIENT_CMD}" ]]; then
  echo "[3/5] Running client workload command..."
  if [[ "${NUM_THREADS}" != "1" ]]; then
  echo "      Note: num_threads is ignored when client_command is provided."
  fi
  bash -lc "${CLIENT_CMD}" >"${OUTDIR}/client.log" 2>&1 &
  CLIENT_PID=$!
  sleep 0.2
  if ! kill -0 "${CLIENT_PID}" >/dev/null 2>&1; then
    echo "Warning: client command exited immediately."
    echo "         Check ${OUTDIR}/client.log; profiling may have little/no load."
  fi
else
  echo "[3/5] No client command provided. Running built-in default workload with ${NUM_THREADS} thread(s)..."
  export PROFILE_DURATION="${DURATION}"
  export PROFILE_NUM_THREADS="${NUM_THREADS}"
  python3 - <<'PY' >"${OUTDIR}/client.log" 2>&1 &
import os
import random
import socket
import time
import threading

HOST = "127.0.0.1"
PORT = 8000
END_AT = time.time() + float(os.environ.get("PROFILE_DURATION", "15"))
NUM_THREADS = max(1, int(os.environ.get("PROFILE_NUM_THREADS", "1")))
PREGEN_BATCH_PER_THREAD = max(1, int(os.environ.get("PROFILE_PREGEN_BATCH_PER_THREAD", "50000")))

def generate_requests(thread_id: int, count: int):
  rng = random.Random(12345 + thread_id)
  next_order_id = thread_id * 1_000_000 + 1
  open_ids = []
  requests = []

  for _ in range(count):
    order_type = rng.choices(["NEW", "MODIFY", "CANCEL"], weights=[0.75, 0.2, 0.05])[0]
    symbol = rng.choice(["NVDA", "AAPL", "TSLA"])
    side_tag = "1" if rng.choice(["BUY", "SELL"]) == "BUY" else "2"

    if order_type == "NEW" or not open_ids:
      payload = (
        f"8=FIX.4.2|35=D|11={next_order_id}|55={symbol}|54={side_tag}|"
        f"44={rng.randint(90000, 110000)}|38={rng.randint(1, 10)}|"
      )
      open_ids.append(next_order_id)
      next_order_id += 1
    elif order_type == "MODIFY":
      oid = rng.choice(open_ids)
      payload = (
        f"8=FIX.4.2|35=G|11={oid}|55={symbol}|54={side_tag}|"
        f"44={rng.randint(90000, 110000)}|38={rng.randint(1, 10)}|"
      )
    else:
      oid = rng.choice(open_ids)
      payload = f"8=FIX.4.2|35=F|11={oid}|"
      open_ids.remove(oid)

    requests.append(payload.encode())

  return requests

thread_requests = [generate_requests(t_id, PREGEN_BATCH_PER_THREAD) for t_id in range(NUM_THREADS)]

def worker(thread_id: int) -> None:
  requests = thread_requests[thread_id]
  idx = 0

  with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))

    while time.time() < END_AT:
      msg = requests[idx]
      idx += 1
      if idx == len(requests):
        idx = 0
      s.sendall(msg)
      s.recv(4096)

threads = []
for t_id in range(NUM_THREADS):
  t = threading.Thread(target=worker, args=(t_id,))
  t.start()
  threads.append(t)

for t in threads:
  t.join()
PY
  CLIENT_PID=$!
fi

echo "[4/5] Recording CPU samples for ${DURATION}s..."
set +e
"${PERF_BIN}" record -F 999 -g -p "${SERVER_PID}" -- sleep "${DURATION}" >"${OUTDIR}/perf-record.log" 2>&1
PERF_EXIT=$?
set -e

if [[ -n "${CLIENT_PID}" ]]; then
  wait "${CLIENT_PID}" || true
fi

if [[ ${PERF_EXIT} -ne 0 ]]; then
  echo "Error: perf record failed."
  echo "See ${OUTDIR}/perf-record.log"
  echo "Common fixes:"
  echo "  sudo sysctl kernel.perf_event_paranoid=1"
  echo "  sudo sysctl kernel.kptr_restrict=0"
  exit ${PERF_EXIT}
fi

echo "[5/5] Generating text report..."
"${PERF_BIN}" report --stdio --sort=symbol >"${OUTDIR}/perf-report.txt"

echo "Done. Artifacts:"
echo "  ${OUTDIR}/perf.data"
echo "  ${OUTDIR}/perf-report.txt"
echo "  ${OUTDIR}/server.log"
echo "  ${OUTDIR}/client.log"
