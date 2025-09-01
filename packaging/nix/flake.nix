{
  description = "NEURALWARE-X";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.05";
  outputs = { self, nixpkgs }: let
    pkgs = import nixpkgs { system = "x86_64-linux"; };
  in {
    packages.x86_64-linux.neuralwarex = pkgs.stdenv.mkDerivation {
      pname = "neuralwarex";
      version = "1.2.0";
      src = ./.;
      nativeBuildInputs = [ pkgs.cmake pkgs.pkg-config ];
      buildInputs = [ ];
      installPhase = ''
        cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
        cmake --build build --target install
      '';
    };
    defaultPackage.x86_64-linux = self.packages.x86_64-linux.neuralwarex;
  };
}
