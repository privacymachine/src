#!/bin/bash

mkdir /pm/logs
echo startup done at `date -u` > /pm/logs/x_started.log

# start firefox in current X
/usr/bin/firefox
