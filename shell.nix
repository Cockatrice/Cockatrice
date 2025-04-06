{ pkgs ? import <nixpkgs> {} }:
  pkgs.mkShell {    
    nativeBuildInputs = with pkgs.buildPackages; [
      # Build tools
      cmake
      cmake-format
      ninja
      bash
      curl
      git
      qtcreator

      # Debug / Test 
      valgrind
      gdb
      clang-tools

      # Compiler
      gcc

      # Libraries
      openssl
      protobuf
      qt6.qtbase
      qt6.full
      qt6.wrapQtAppsHook
    ];

    # Make debug builds work
    # https://github.com/NixOS/nixpkgs/issues/18995
    hardeningDisable = [ "fortify" ];
  }
