#!/bin/bash
# create the patch file of the sailfish sdk
if [ -d ./RemoteDisplay ]; then
  pushd RemoteDisplay
  git diff > ../RemoteDisplay_changes.diff
  popd
fi

