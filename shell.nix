{ pkgs ? import <nixpkgs> {} }:
  pkgs.mkShell {    
    nativeBuildInputs = with pkgs.buildPackages; [
      # Build tools
      cmake
      cmake-format
      bash
      curl
      git
      qtcreator

      # Debug / Test 
      valgrind
      gdb

      # Compiler
      gcc

      # Libraries
      openssl
      protobuf
      qt6.qtbase
      qt6.full
      qt6.wrapQtAppsHook
    ];
  }
