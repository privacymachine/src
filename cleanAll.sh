#!/bin/bash

echo "clean content of the doc_generated dir"
if [ -d doc_generated ]; then
  rm -rf doc_generated/*
fi

./cleanPm.sh
./cleanExtLibs.sh
