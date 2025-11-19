#!/usr/bin/env python3

"""
Script for building Micropython with the `diff_drive` module.
"""

import multiprocessing
import subprocess
import shutil
import sys
import os
import os.path as path
import argparse


MICRO_PYTHON_DIR = "micropython"
PICO_SDK_DIR     = "pico-sdk"
USER_MODULE_DIR  = "stepper_module"


ROOT_MARKER_FILE = "build.py"

ROOT = path.abspath(path.dirname(__file__))

RP_DIR = path.join(ROOT, MICRO_PYTHON_DIR, "ports", "rp2")
BUILD_DIR = path.join(ROOT, "build")

# Colors
BLUE  = "\033[94m"
RED   = "\033[91m"
RESET = "\033[0m"

def ensure_exe_installed(exes: str | list[str]) -> str:
    if isinstance(exes, str): exes = [exes]

    missing = []

    for exe in exes:
        path = shutil.which(exe)
        if path is None:
            missing.append(exe)

    if len(missing) > 0:
        print(f"\n{RED}Error: Missing required executables:{RESET}")
        for exe in missing:
            print(f" - {exe}")
        exit(1)

def cmd(command: str | list[str], cwd: str | None = None, ignore_fail=False, **kwargs) -> subprocess.CompletedProcess:
    if isinstance(command, list):
        if len(command) == 0: raise ValueError("Command list cannot be empty")
        ensure_exe_installed(command[0])

    print(f"{BLUE}[CMD] {' '.join(command) if isinstance(command, list) else command} {'(in ' + cwd + ')' if cwd else ''}{RESET}")

    if isinstance(command, str):
        kwargs.setdefault("shell", True)

    res = subprocess.run(command, cwd=cwd, **kwargs)

    if res.returncode != 0 and not ignore_fail:
        print(f"\nError: Command failed with exit code {res.returncode}")
        sys.exit(res.returncode)

    return res


def download_micropython():
    if path.exists(MICRO_PYTHON_DIR): return
    cmd(["git", "clone", "https://github.com/micropython/micropython.git", MICRO_PYTHON_DIR])

def download_pico_sdk():
    if path.exists(PICO_SDK_DIR): return
    cmd(["git", "clone", "https://github.com/raspberrypi/pico-sdk", PICO_SDK_DIR])


def build_micropython():
    ensure_exe_installed(["make", "arm-none-eabi-gcc"])


    cmd(["make", "submodules"], cwd=RP_DIR)

    cmd(["make",
         f"USER_C_MODULES=\"{path.join(ROOT, USER_MODULE_DIR)}\"",
         '-j', str(multiprocessing.cpu_count() - 1),
         ], cwd=RP_DIR)

    if path.exists(BUILD_DIR):
        shutil.rmtree(BUILD_DIR)

    shutil.copytree(path.join(RP_DIR, "build-RPI_PICO"), BUILD_DIR)


def clean_micropython():
    cmd(["make", "clean"], cwd=RP_DIR, ignore_fail=True)
    if path.exists(BUILD_DIR): shutil.rmtree(BUILD_DIR)

def flash_micropython():
    ensure_exe_installed(["picotool"])

    uf2_path = path.join(BUILD_DIR, "firmware.uf2")

    print("Flashing firmware to PICO requires sudo privileges. "
          "Please review the script before proceeding.")

    cmd(["sudo", "picotool", "load", uf2_path])


def main():
    parser = argparse.ArgumentParser(description=sys.modules[__name__].__doc__)
    parser.add_argument("--clean", action="store_true", help="Clean build artifacts before building")
    parser.add_argument("--rebuild", action="store_true", help="Clean and rebuild Micropython")
    parser.add_argument("--flash", action="store_true", help="Flash the built firmware to the PICO after building")

    args = parser.parse_args()

    if args.clean:
        clean_micropython()
        return

    if args.rebuild:
        clean_micropython()

    download_micropython()

    build_micropython()

    if args.flash:
        flash_micropython()


if __name__ == "__main__":
    main()
