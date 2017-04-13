# Concept of BaseDisk creation
### For implementation and HOWTO look at ../packaging/BaseDisk/README.md
## Script libBaseDiskBuild.py

**Purpose: Build a new BaseDisk from scratch**

### Program flow
* replace old flat.vmdk with zeros
* define libvirt build VM as following
    * customized grml.iso as boot CD
    * flat.vmdk as sata disk 
* define python VM handler
* boot VM and bootstrap debian
* install packages
* shutdown VM and undefine it in libvirt
* define libvirt build VM as following
    * flat.vmdk as booting sata disk
    * VirtualBox guest additions as CD
* install guest additions
* shutdown VM and calculate cryptographic hash and signs it

## Script checkForBaseDiskUpdates.py 

**Purpose: check for new updates from apt-repositories (script called via cron)**
**not existing now**

### Program flow
* boots last released BaseDisk
* check for updates: apt, tor, (browser-plugins), (vpn-configurations)
* creates a text based report

## Script report-changes.py
**Purpose: bases on a whitelist create a report what's new -> send mail to release managers**
**not existing now**

## Script buildNewBaseDisk.py 
**Purpose: triggerd by maintainers to build a new basedisk and prepare for release**

* triggers script libBaseDiskBuild.py
* create 
    * checksums, 
    * pgp-signature, 
    * librsync-diff to all previous BaseDisk versions

## Checklist CreateANewRelease.md
contains all needed manual steps like testing to be done by the Release-Mangers

## Script deployNewRelease.py
**Purpose: do all needed stuff that the Users can update the new version**
**not existing now**

Can be a combination of
* BaseDisk
* config
* executable

Needs the ReleaseNotes.md as parameter

Executes the following steps:
1. build a new local version of update-blog.xml
2. do the upload (BaseDisk-zip, config-zip, exe-zip)
3. verify the upload
4. copy update-blog.xml to web01
5. (update Homepage-Download-Links)

