#!/bin/bash

# this scripts waits till the user is (auto-)logged in into the desktop
# The script /pm/scripts/autostart_after_x_login.sh which is started in /home/pm/.config/autostart/pm_startup.desktop is started to early, we habe to wait till a process exists

until pidof obex-data-server
do
  echo not running, wait a second and check again...
  sleep 1
done

echo give em 5 additional seconds
sleep 5

echo X-Desktop is started sucessfully.

