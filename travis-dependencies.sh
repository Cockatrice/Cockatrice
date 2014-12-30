#!/bin/bash

if [[ $TRAVIS_OS_NAME == "osx" ]] ; then
  brew update
  if (( QT4 )); then
    brew install qt protobuf libgcrypt
  else
    brew install qt5 protobuf libgcrypt
  fi
else
  if (( QT4 )); then
    sudo apt-get update -qq
    sudo apt-get install -y qtmobility-dev libprotobuf-dev protobuf-compiler libqt4-dev
  else
    sudo add-apt-repository --yes ppa:canonical-qt5-edgers/qt5-beta-proper
    sudo apt-get update -qq
    sudo apt-get install -y libprotobuf-dev protobuf-compiler qtbase5-dev cmake\
      qtdeclarative5-dev libqt5webkit5-dev libsqlite3-dev qt5-default qttools5-dev-tools\
      qttools5-dev libqt5svg5-dev libqt5svg5 qtbase5-dev libqt5gui5 libqt5printsupport5 \
      libqt5widgets5 qt5-qmake qtbase5-dev-tools qtchooser libqt5quick5 libqt5quickparticles5 \
      libqt5quicktest5 libqt5designer5 libqt5designercomponents5 libqt5gui5 libqt5help5
  fi
fi