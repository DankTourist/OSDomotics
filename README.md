# Operating Systems Project 2026-2 - Domotics

This repository contains the implementation of "Project 2026-2 - Domotics" for the Operating Systems course. The project consists of a home automation system emulator written in **C and Bash**.

Each device in the system is represented by a distinct OS-level process. The process architecture is **flat**: all devices are direct children of a single main process called `Controller`. However, the system supports a **logical hierarchy** (control devices managing other devices) maintained and routed exclusively via Inter-Process Communication (IPC).

---

## Prerequisites

The project is designed to be compiled and executed in a **Linux** environment (tested on Ubuntu 24.04).
The following are required:
* `gcc` compiler (tested on GCC 15.2)
* `make` (if using the Makefile)
* `bash`

---

## Compilation and Execution

The project includes an automated system for compiling and cleaning the environment, manageable via `Makefile` (or via the `./build.sh` script).

### Available Commands:

* **Compilation:**
  ```bash
  make build
  # or: ./build.sh build
