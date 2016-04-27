#! /bin/bash

# select 10 random font packages in 
# <font_package_list_jessie_apt_install.txt> and install them

FONT_COUNT=10
if [[ "$1" != "" ]]; then
    FONT_COUNT=$1
fi

cd /pm_on_restore/fonts
PACKAGES="$(sort -R font_package_list_jessie_apt_install.txt | head -n $1 | xargs echo)"

apt-get install -y ${PACKAGES}

fc-cache

echo $PACKAGES
