{
  description = "Flake utils demo";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = { nixpkgs, flake-utils, ... }:
    flake-utils.lib.eachDefaultSystem (system:
        let
            pkgs = nixpkgs.legacyPackages.${system};
        in
        {
            devShells.default = pkgs.mkShell {
                buildInputs = with pkgs; [
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
