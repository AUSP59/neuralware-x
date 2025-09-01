
{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  buildInputs = [ pkgs.cmake pkgs.ninja pkgs.gcc13 pkgs.llvmPackages_17.clang pkgs.doxygen pkgs.graphviz pkgs.lcov pkgs.gcovr ];
}
