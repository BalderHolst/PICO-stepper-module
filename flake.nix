{
  description = "Flake utils demo";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";

  outputs = { nixpkgs, flake-utils, ... }:
    flake-utils.lib.eachDefaultSystem (system:
        let
            pkgs = import nixpkgs { inherit system; };
        in
        {
            devShells.default = pkgs.mkShell {
                buildInputs = with pkgs; [
                    mpremote              # MicroPython CLI remote control tool
                    thonny                # IDE for MicroPython
                    minicom               # Serial monitor
                    picotool              # Raspberry Pi Pico utility
                    cmake                 # Build system
                    clang-tools           # C/C++ tools
                    gcc-arm-embedded-14   # ARM Embedded GCC toolchain
                    pkgsi686Linux.gcc     # 32-bit GCC toolchain
                ];
            };
        }
    );
}
