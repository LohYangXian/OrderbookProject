# Windows Setup and Run Guide (WSL + Ubuntu)

This guide sets up the project on Windows using WSL2 (Ubuntu), then builds, tests, and runs the server and Python client.

Why WSL?
- The server code uses POSIX/Linux socket headers and APIs.
- WSL2 gives a native Linux environment on Windows with minimal source changes.

## 1) Prerequisites

- Windows 10 (22H2+) or Windows 11
- Administrator access for initial setup
- VS Code installed

Optional but recommended:
- Enable hardware virtualization in BIOS/UEFI if disabled

## 2) Install WSL and Ubuntu

Open PowerShell as Administrator:

```powershell
wsl --install -d Ubuntu-22.04
```

Reboot if prompted.

After reboot, launch Ubuntu from Start menu and complete first-time setup:
- Create Linux username
- Create Linux password

Verify installation in PowerShell:

```powershell
wsl -l -v
```

Expected output includes:
- Ubuntu-22.04
- VERSION = 2

Set Ubuntu-22.04 as default distro (optional but recommended):

```powershell
wsl -s Ubuntu-22.04
```

## 3) Install VS Code Extensions

In VS Code, install:
- Remote - WSL (ms-vscode-remote.remote-wsl)
- C/C++ (ms-vscode.cpptools)
- Python (ms-python.python)
- Bazel (optional, for BUILD file UX)

## 4) Open This Project in WSL

Open Ubuntu terminal and run:

```bash
cd /mnt/{main drive}/OrderbookProject
code .
```

Confirm VS Code status bar shows: WSL: Ubuntu-22.04

## 5) Install Linux Build Tools and Python

Run in Ubuntu terminal:

```bash
sudo apt update
sudo apt install -y build-essential clang gdb lldb git curl unzip zip pkg-config python3 python3-venv python3-pip npm
```

## 6) Install Bazel (Bazelisk)

This repo is pinned via `.bazelversion`, so use Bazelisk:

```bash
sudo npm install -g @bazel/bazelisk
bazel version
```

Expected: Bazel command works and resolves the version in `.bazelversion`.

## 7) Build and Test the C++ Project

From repo root:

```bash
cd /mnt/{main drive}/OrderbookProject
bazel build //src:main_server
bazel test //tests:orderbook_test
```

If both succeed, your C++ setup is complete.

## 8) Run the Server (main_server.cpp from WSL)

In terminal 1 (WSL Ubuntu terminal or VS Code integrated terminal in a WSL window):

```bash
cd /mnt/{main drive}/OrderbookProject
bazel run //src:main_server
```

Expected log:
- Server listening on port 8000

Keep this terminal open while running the Python client or notebook.

## 9) Run `client.ipynb` in VS Code with a WSL Python Kernel

In terminal 2:

```bash
cd /mnt/{main drive}/OrderbookProject/py_client
python3 -m venv .venv
source .venv/bin/activate
python -m pip install --upgrade pip
python -m pip install numpy matplotlib ipykernel jupyter
python -m ipykernel install --user --name orderbook-wsl --display-name "Python (orderbook-wsl)"
```

Then in VS Code (connected to WSL):
- Open `py_client/client.ipynb`
- Click **Select Kernel** (top-right in the notebook editor)
- Choose `Python Environments` -> `.venv/bin/python`, or choose `Jupyter Kernels` -> `Python (orderbook-wsl)`
- Run notebook cells

Notes:
- Start the server first (`bazel run //src:main_server`) before running notebook cells.
- If plotting windows are unavailable, metrics still print in notebook output.

## 10) Run the Python Script Client (optional)

In terminal 2:

```bash
cd /mnt/{main drive}/OrderbookProject/py_client
python3 -m venv .venv
source .venv/bin/activate
pip install --upgrade pip
pip install numpy matplotlib
python client.py
```

Expected output:
- RTT metrics printed for ALL/NEW/MODIFY/CANCEL
- Histograms displayed if GUI forwarding is available

If plots cannot display in terminal-only environment, metrics will still print.

## 11) Common Commands

Build:

```bash
bazel build //src:main_server
```

Run server:

```bash
bazel run //src:main_server
```

Run tests:

```bash
bazel test //tests:orderbook_test
```

Run notebook dependencies setup (WSL):

```bash
cd /mnt/{main drive}/OrderbookProject/py_client
source .venv/bin/activate
python -m pip install -U numpy matplotlib ipykernel jupyter
```

Clean Bazel cache (only if needed):

```bash
bazel clean --expunge
```

## 12) Troubleshooting

### A) `wsl -l -v` shows no distro

Install explicitly:

```powershell
wsl --install -d Ubuntu-22.04
```

### B) `WslRegisterDistribution failed ... 0x80070002`

Try:

```powershell
wsl --update
wsl --shutdown
wsl --install -d Ubuntu-22.04
```

If still failing, reboot and retry.

### C) `code .` does not open from Ubuntu

- Ensure VS Code is installed on Windows
- Ensure Remote - WSL extension is installed
- Reopen Ubuntu and run `code .` again

### D) Permission denied during package install

- Use `sudo` and enter your Ubuntu password

### E) Build works but server/client cannot connect

- Ensure server is running first
- Confirm port 8000 is free
- Keep server terminal open while running client

### F) Notebook kernel does not show in VS Code

- Confirm VS Code status bar says `WSL: Ubuntu-22.04`
- Re-select interpreter with `Python: Select Interpreter` and choose `.venv/bin/python`
- Re-run kernel registration:

```bash
cd /mnt/{main drive}/OrderbookProject/py_client
source .venv/bin/activate
python -m ipykernel install --user --name orderbook-wsl --display-name "Python (orderbook-wsl)"
```

- Reload VS Code window and try **Select Kernel** again
