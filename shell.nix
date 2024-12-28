{ pkgs ? import <nixpkgs> {} }:
  pkgs.mkShell {    
    nativeBuildInputs = with pkgs.buildPackages; [
      # Build tools
      cmake
      cmake-format
      bash
      curl
      git

      # Test
      valgrind
      gdb

      # Compiler
      gcc

      # Libraries
      openssl
      libsForQt5.full
      protobuf
    ];
  }
