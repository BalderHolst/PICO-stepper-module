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
USER_MODULE_DIR  = "diff_drive_module"

ROOT_MARKER_FILE = "build.py"

ROOT = path.abspath(path.dirname(__file__))

RP_DIR = path.join(ROOT, MICRO_PYTHON_DIR, "ports", "rp2")

# Colors
BLUE = "\033[94m"
RESET = "\033[0m"

def ensure_exe_installed(exes: str | list[str]) -> str:
    if isinstance(exes, str): exes = [exes]

    missing = []

    for exe in exes:
        path = shutil.which(exe)
        if path is None:
            missing.append(exe)

    if len(missing) > 0:
        print("Error: Missing required executables:")
        for exe in missing:
            print(f" - {exe}")

def cmd(command: str | list[str], cwd: str | None = None, **kwargs) -> subprocess.CompletedProcess:
    if isinstance(command, list):
        if len(command) == 0: raise ValueError("Command list cannot be empty")
        ensure_exe_installed(command[0])

    print(f"{BLUE}[CMD] {' '.join(command) if isinstance(command, list) else command} {'(in ' + cwd + ')' if cwd else ''}{RESET}")

    if isinstance(command, str):
        kwargs.setdefault("shell", True)

    return subprocess.run(command, cwd=cwd, **kwargs)


def download_micropython():
    ensure_exe_installed(["git"])
    print("Cloning MicroPython repository...")
    print("TODO: implement git clone")


def build_micropython():
    ensure_exe_installed(["make", "arm-none-eabi-gcc"])


    cmd(["make", "submodules"], cwd=RP_DIR)

    cmd(["make",
         f"USER_C_MODULES=\"{path.join(ROOT, USER_MODULE_DIR)}\"",
         '-j', str(multiprocessing.cpu_count() - 1),
         ], cwd=RP_DIR)

def clean_micropython():
    cmd(["make", "clean"], cwd=RP_DIR)

def flash_micropython():
    ensure_exe_installed(["picotool"])

    uf2_path = path.join(RP_DIR, "build-RPI_PICO", "firmware.uf2")

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
