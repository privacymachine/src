#!/bin/bash

cd /pm

TORBROWSERVERSION=$(wget -q --no-check-certificate -O - https://www.torproject.org/download/download.html | grep -o -E '/[0-9a-z\.]+/tor-browser-linux64-[0-9a-z\.]+_en-US\.tar\.xz' | uniq)

if [ $? -ne 0 ] ; then
    echo FAILED to download torbrowser version
    exit 1 
fi

TORBROWSERARCHIVE=$(echo $TORBROWSERVERSION | awk -F '/' '{print $3}')


TORBROWSERURL='https://www.torproject.org/dist/torbrowser'$TORBROWSERVERSION

if [ $# -eq 1 ]
then
	TORBROWSERPROXIE=$1'www.torproject.org/dist/torbrowser'$TORBROWSERVERSION
else
	TORBROWSERPROXIE=$TORBROWSERURL
fi


wget -q $TORBROWSERPROXIE  
if [ $? -ne 0 ] ; then
    echo FAILED to downloaded torbrowser
    exit 1
fi

wget -q --no-check-certificate $TORBROWSERURL'.asc'
if [ $? -ne 0 ] ; then
    echo FAILED to downloaded torbrowser signature
    exit 1
fi


gpg  --batch --keyserver x-hkp://pool.sks-keyservers.net --recv-keys EF6E286DDA85EA2A4BA7DE684E2C6E8793298290 
gpg  --batch --verify $TORBROWSERARCHIVE'.asc' $TORBROWSERARCHIVE

if [ $? -ne 0 ] ; then
    echo FAILED to verify signatur of torbrowser!
    exit 1
fi

tar -Jxf $TORBROWSERARCHIVE

if [ $? -ne 0 ] ; then
    echo FAILED to inflate torbrowser archive
    exit 1
fi

rm $TORBROWSERARCHIVE $TORBROWSERARCHIVE'.asc'

if [ $? -ne 0 ] ; then
    echo FAILED to cleanup torbrowser archive file
    exit 1
fi

echo Installed $TORBROWSERARCHIVE
exit 0



