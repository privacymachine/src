#!/bin/bash

if [ -d ./FreeRDP ]; then
  rm -rf FreeRDP
fi

if [ -d ./RemoteDisplay ]; then
  rm -rf RemoteDisplay
fi

if [ -d ./libsodium ]; then
  rm -rf libsodium
fi
