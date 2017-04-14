#!/bin/bash


startTime=`date`

pushd build_pm
#ctest --verbose
#ctest --output-on-failure 
ctest --no-compress-output -T Test || /usr/bin/true
popd

echo "UnitTests started at: $startTime" 
echo "UnitTests finished at: `date`"
