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
                    thonny
                    minicom
                    micropython
                    cutecom
                    cmake
                    clang-tools
                    picotool
                    gcc-arm-embedded-14
                    pkgsi686Linux.gcc
                ];
            };
        }
    );
}
