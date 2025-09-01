{
  description = "NEURALWARE-X dev shell and package";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.05";
  outputs = { self, nixpkgs }:
    let
      forAllSystems = f: nixpkgs.lib.genAttrs [ "x86_64-linux" "aarch64-linux" ];
    in rec {
      packages = forAllSystems (system:
        let pkgs = import nixpkgs { inherit system; };
        in {
          default = pkgs.stdenv.mkDerivation {
            pname = "neuralware-x";
            version = "1.0.0";
            src = ./.;
            nativeBuildInputs = [ pkgs.cmake pkgs.ninja ];
            buildPhase = ''
              cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
              cmake --build build --parallel
            '';
            installPhase = ''
              cmake --install build --prefix $out
            '';
          };
        });
      devShells = forAllSystems (system:
        let pkgs = import nixpkgs { inherit system; };
        in {
          default = pkgs.mkShell {
            packages = with pkgs; [ cmake ninja gcc gdb clang clang-tools bear ccache pkg-config python3 gcovr ];
            shellHook = ''
              export CCACHE_DIR=$PWD/.ccache
              echo "Dev shell ready: cmake+ninja+clang-tools"
            '';
          };
        });
    };
}
