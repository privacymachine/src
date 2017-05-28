# PrivacyMachiene adapted firefox profile


## This File describes what's in this directory.

### DEV.md
Notes and links for development as well for creating a build guide 

### testprofile/

A ff profile to test and develop 

### xulstore.json
In this file the only option set is _"sizemode": "maximized"_ which tells firefox to start in fullscreen mode, this works.

### user.js
In this file a lot of settings in firefox are overwritten to protect the users privacy and disable probaply insecure firefox features.
copied from _https://github.com/pyllyukko/user.js/tree/relaxed_

### prefs.js
this file holds the user preferences of firefox  
see _https://developer.mozilla.org/en-US/docs/Mozilla/Preferences/A_brief_guide_to_Mozilla_preferences_ 7
  
__the numer after prefs.js is only a version number fordeveloping__  


#### prefs.js.0
Initial prefs.js after a clean ff start

#### prefs.js.1
a prefs.js with blanc startpage

#### prefs.js.2
a cleand profile with comments and disabled telemetry

#### prefs.js.3
prefs.js.2 + disabled third party cookies

#### prefs.js.4
prefs.js.3 + send DoNotTrack header

#### prefs.js.5
prefs.js.4 + disabled sending any referrer 