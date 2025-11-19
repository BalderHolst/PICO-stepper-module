{
  description = "Flake for MicroPython development on Raspberry Pi Pico";
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
                    mpremote           # MicroPython CLI remote control tool
                    minicom            # Serial monitor
                    picotool           # Raspberry Pi Pico utility
                    cmake              # Build system
                    gcc-arm-embedded   # ARM Embedded GCC toolchain
                ];
            };
        }
    );
}
