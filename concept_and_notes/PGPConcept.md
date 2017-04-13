# How PGP Keys are used by the PrivacyMachine

An offline Key created which is never connected to the Internet and is the root for the Web of Trust.
This key signs all other Keys which are used by the PrivacyMachine-Team.
Team-Members also signs the Offline-Key

** PrivacyMachine offline-master-key (used as primary soure of trust) [Sign]**
signes the generic keys:
* PrivacyMachine build-key (used in automated build process) [Sign]
* PrivacyMachine release-key (used by release-managers to authorize a release) [Sign]
* PrivacyMachine developers pm-dev@privacymachine.eu [Sign/Encrypt]
* PrivacyMachine team contact@privacymachine.eu [Sign/Encrypt]
  
signs personal keys i.e.:
* alex@privacymachine.eu
* olaf@privacymachine.eu
* bernhard@privacymachine.eu
* ...

 # TODO: Add description which Key is used for what