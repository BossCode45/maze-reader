{
    description = "default c++ flake";
    inputs = {
        nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.11";
        flake-utils.url = "github:numtide/flake-utils";
    };
    outputs = { self, nixpkgs, ... }@inputs:
        inputs.flake-utils.lib.eachDefaultSystem (system: let
            pkgs = nixpkgs.legacyPackages.${system};
        in {
            devShells.default = pkgs.mkShell {
                nativeBuildInputs = [
                    pkgs.gcc
                    pkgs.gnumake
                    pkgs.clang-tools
                ];
            };
        });
}
