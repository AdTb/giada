#!/usr/bin/env bash

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

	: # null command - nothing to do 

elif [[ $TRAVIS_OS_NAME == 'linux' ]]; then

  sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y # for gcc 6
  sudo apt-get update -qq

fi