#!/bin/bash

# usage: 
#  first param: display width
#  second param: display heigth

modeline=`cvt $1 $2 | tail --lines 1 | sed 's@Modeline @@'`
modeshort=`echo $modeline | cut --fields 1 --delimiter ' '`

echo change display resulution to $modeshort
xrandr --newmode $modeline > /dev/null 2>&1
xrandr --addmode VBOX0 $modeshort
xrandr --output VBOX0 --mode $modeshort
