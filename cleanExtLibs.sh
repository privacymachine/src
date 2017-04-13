#!/bin/bash

echo "clean content of the build_ext_libs dir"
if [ -d build_ext_libs ]; then
  rm -rf build_ext_libs/*
fi

echo "remove files from working_dir..."
rm -f working_dir/lib/*

# Windows?
#rm -f working_dir/build_libs/*

