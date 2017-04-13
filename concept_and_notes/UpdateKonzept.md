# PrivacyMachine Update-Konzept

Der Updateserver ist konfigurierbar - damit haben z.B Uni's oder paranoide User selbst die Möglichkeit den ISO-Update-Schritt in die Hand zu nehmen 
und von uns nur das PM-Executable zu verwenden.

Updates werden über ein RSS-Feed im XML format ausgeliefert:
UpdateServer = https://privacymachine.eu/autoupdate.xml
UpdateServer = http://143.23.12.12/autoupdate.xml
UpdateServer = file:///home/jondoe/pm_customised_update/autoupdate.xml

## Requirements
Hintergrundinfo siehe u.A. _Inkrementelle Updates_.

* Auslieferung muss über file/ftp-Share möglich sein (kein Rsync-Server)
* Server muss komprimiertes diff (abhängig von der aktuellen Clientversion) bereits erstellt haben.
* Client soll nichts berechnen müssen sondern nur das für ihn passende Update-Diff-File herunterladen und auf seine Basis anwenden können.
* Eigene log-Datei für Update, fortlaufend Ausgaben aller Updates speichern
* Anzeige: Update-Status (Erfolgreich/Fehlgeschlagen), Changelog (bei manual Deployment)
  - Changelog aus Feed herunterladen und in QTextEdit setzen
* VM-Clean Skript (saubere, leere Umgebung schaffen), basierend auf intakter BaseDisk alle VMs neu
  erstellen.
* Während Update-Vorgang soll der PMCommandExecutor pausierbar sein (Pause-Knopf in
  WidgetCommandExec, mit dem PMCommandExecutor dann nach nächstem CommandFinished pausiert).

## Versionsnummern
Um mit den Versionierungsschemata lt. Linux und Windows kompatibel zu bleiben, werden Versionen wie
folgt nummeriert:

### BaseDisk-Versioning:
    <Major>.<Minor>.<ComponentMajor>.<ComponentMinor>
     |          |     |               |
     |          |     |             not used (always 0)
     |          |     |                      
     |          |   changes after build basedisk was necessary (starts with 1)
     |          |
     |        changes on incompatible new Features (API-Change to BaseDisk)
     |        (starts with 9 (next: 10))
     |
    Mainly for marketing purpose (Beta: start with 0)

### PrivacyMachine-Versioning:
    <Major>.<Minor>.<ComponentMajor>.<ComponentMinor>
     |          |     |               |
     |          |     |         changes on Bugfix(es) 
     |          |     |
     |          |   changes on new Features
     |          |     
     |          |
     |        changes on incompatible new Features (API-Change to BaseDisk)
     |        (starts with 9 (next: 10))
     |
    Mainly for marketing purpose (Beta: start with 0)

### Config-Versioning:
    <Major>.<Minor>.<ComponentMajor>.<ComponentMinor>
     |          |     |               |
     |          |     |         changes on Bugfix(es) 
     |          |     |
     |          |   changes on each version
     |          |     
     |        same as PM
     |        
    same as PM
   

Die jeweiligen Nummern sind monoton steigend. Die zweite Stelle (API) zeigt den Endanwendern an, ob
es inkompatible Änderungen gegeben hat.

Beispiele:
* API 4, PrivacyMachine 13, für Windows: `1.4.13.0` .0=PM-Bugfix Setup=PrivacyMachine-1.4.13.msi
* API 2, PrivacyMachine 11, für Linux: `1.2.11.0`
* API 2, BaseDisk 4_3, für Linux: `1.2.4.3`

## Update-Konzept des Basis-Images am Server ##
* /var ist eine extra (virtuelle) Disk und wird bei der auslieferung im /etc/fstab wieder entfernt
   -> Dadurch wird z.B. /var/log und /var/cache/apt nicht ausgeliefert
* Die Größe des virtuellen Root-Partition ist möglichst klein (weil sie 1:1 beim User liegt)
* Mittels Rdiff und zip (zlib) werden die Differenz-Files auf die User-Versionen erzeugt.

Alle genannten Files werden als Zip ausgeliefert:


    01  BaseDisk_0  
    
    02  BaseDisk_1 = BaseDisk_0 + BaseDisk_delta_0-1  
    
    03  BaseDisk_2 = BaseDisk_1 + BaseDisk_delta_1-2 
            || BaseDisk_0 + BaseDisk_delta_0-2  
            
    04  BaseDisk_3 = BaseDisk_2 + BaseDisk_delta_2-3 
            || BaseDisk_1 + BaseDisk_delta_1-3 
                || BaseDisk_0 + BaseDisk_delta_0-3  
                
    05  BaseDisk_4 = BaseDisk_3 + BaseDisk_delta_3-4 
            || BaseDisk_2 + BaseDisk_delta_2-4 
                || BaseDisk_1 + BaseDisk_delta_1-4 
                    || BaseDisk_0 + BaseDisk_delta_0-4  
                    
    06  BaseDisk_5 = BaseDisk_4 + BaseDisk_delta_4-5
            || BaseDisk_3 + BaseDisk_delta_3-5 
                || BaseDisk_2 + BaseDisk_delta_2-5 
                    || BaseDisk_1 + BaseDisk_delta_1-5 
                        || BaseDisk_0 + BaseDisk_delta_0-5
                    
    07  BaseDisk_6 = BaseDisk_5 + BaseDisk_delta_5-6  
            || BaseDisk_4 + BaseDisk_delta_4-6 
                || BaseDisk_3 + BaseDisk_delta_3-6 
                    || BaseDisk_2 + BaseDisk_delta_2-6 
                        || BaseDisk_1 + BaseDisk_delta_1-6
                            ||  BaseDisk_0 -> download BaseDisk_6

**In diesem Beispiel ist das Vorhalteintervall 5 BaseDisks ** 


### Deployment concept

**Legende der benutzten Versionsnummer-Abkürzungen:
    
    W ----- Major (Marketing)
    X ----- BaseDisk minor (API) 
    Y, Z -- BaseDisk component major
    M ----- executable/config Minor
    N ----- executable ComponentMajor
    P ----- executable ComponentMinor
    Q ----- config ComponentMajor
    R ----- config ComponentMinor

#### server folder structure

    - release/ or unstable/
    | appcast.xml
    | RelaseNotes_DE.html
    | RelaseNotes_EN.html
    |
    - W/ ... Marketing
    | |
    | - base_disk/
    | | - X/ ... API
    | |   | BaseDisk_Z.zip ... Y, Z = Component Major
    | |   | BaseDisk_delta_Y-Z.zip
    | |
    | - M/ ... Minor
    | | - config/
    | | | | config_Q_R.zip ... Q_R = Component Major _ Component Minor
    | | |
    | | - install/
    | | | | PrivacyMachine_win64_W_M_N.msi ... W_M_N = Marketing _ Minor _ Component Major
    | | | | PrivacyMachine_debian-jessie_W_M_N.deb
    | | | | 
    | | | - src
    | | | | | PrivacyMachine_W_M_N_P.tar.gz 
    | | | | |   ... W_M_N_P = Marketing _ Minor _ Component Major _ Component Minor
    | | | 
    | | - binary/
    | | | PrivacyMachine_debian-jessie_W_N_M_P.zip
    | | |   ... W_M_N_P = Marketing _ Minor _ Component Major _ Component Minor
    | | | PrivacyMachine_win64_W_M_N_P.zip
        

#### zip-file contents

**Im BaseDisk_Z.zip befindet sich:**
* BaseDisk_Z.flat.vmdk
* BaseDisk_Z.vmdk
* BaseDisk_Z_capabilities.json
* BaseDisk_Z_sha256sums.txt
* BaseDisk_Z_sha256sums.txt.asc


**Im BaseDisk_delta_Y_Z.zip befindet sich:**
* BaseDisk_delta_Y-Z.flat.rdiff
* BaseDisk_Z.vmdk
* BaseDisk_Z_capabilities.json
* BaseDisk_Z_sha256sums.txt
* BaseDisk_Z_sha256sums.txt.asc

**todo: struktur der anderen .zip files auflisten**

### Client:

    update?
       |
     ja:
     download update Files
       |
**TODO!**


Differential Backup in Windows with Delta Files - Using 7-Zip and RDiff:
<http://www.edendevelopments.co.uk/rdiff.php>
  
### Implementierung mittels Fervor
#### Automatisches Herunterladen und Entpacken
Im `autoupdate` Branch kann Fervor updates von selber herunterladen und entpacken, womit die
manuellen Updates hinfällig sind [[2](https://github.com/pypt/fervor/tree/autoupdate#description)].

### Inkrementelle Updates
Folgende Komponenten müssen aktualisiert werden:  
1 PrivacyMachine Binary, inkl. Konfigurationsdateien (Automatisch)  
2 BaseDisk.vdi (Automatisch)  
3 Virtual Box (Manuell - Benutzer wird darauf hingewiesen dass ein Update fällig ist)  

Nach Möglichkeit kommen die Updates für alle Komponenten in einen einzigen App Cast (nur ein
HTTP-Request) - mit einem `<channel>` pro Komponente.

Wahlweise passiert das Ganze unbeaufsichtigt, oder interaktiv (eigener Knopf) in WidgetNewTab,  mit
Abschlussmeldung "Die PrivacyMachine wurde aktualisiert. Die Änderungen werden beim nächsten
Neustart wirksam. Details siehe Changelog:" - je nach Deployment-Einstellung. Changelog unmittelbar
darunter anzeigen.
Snapshots gleich im Hintergrund erstellen => snapshot-Namen erweitern um Version, siehe `vmdk`.

INI Sektion:

    [Update]
    Deployment=manual/automatic/disabled

Nötige Änderungen in Fervor:
* Prüfsummen zu XML hinzufügen
* Implizite gezippte Übertragung von app cast (Client zu Server: "Gib mir ...xml. Ich kann auch ZIP"
  => Server sendet gezipptes XML)
  br - Brotli hat zwar hohe Kompression, aber in NGINX noch nicht einfach verfügbar
  [[1](https://github.com/google/ngx_brotli)]
  => request.setRawHeader("Accept-Encoding", "gzip,deflate");
  [[2](http://stackoverflow.com/questions/2340548/does-qnetworkmanager-get-accept-compressed-replies-by-default)] + quazip
* Mehrere Dateien herunterladen, basierend auf aktueller BaseDisk Version
* rdiff ausführen statt Update drüberspielen

[1] https://github.com/google/ngx_brotli  
[2] http://stackoverflow.com/questions/2340548/does-qnetworkmanager-get-accept-compressed-replies-by-default  


#### Ablauf für PM binary Update unter Windows:  
1 changelog.html Aus app cast herauslesen, lokal speichern
2 PM binary aktualisieren  
3 Neue binary im Hintergrund starten, die neue Snapshots mit BaseDisk erstellt - siehe oben   
4 Neue Konfigurationsdateien herunterladen, neu starten, erst dann neue Konfigurationsdateien übernehmen.    
  Liste zum Umbenenennen/Löschen von Dateien liegt im Update-Teil der neuen Binary.  

Inkrementelle Updates von Binary zu Binary werden ein einer Folge aneinandergereiht. Somit hat man
zwar eine lange Kette von Einzelschritten, die die neue Binary durchführt, aber es ist nach wie vor
nur eine Binary.

#### Ablauf für Konfigurationsdateien:  
1 zip-Datei herunterladen  
2 PM neu starten, diese übernimmt neue Konfigurationsdateien.  

#### Ablauf für BaseDisk app cast:  
1 Client lädt app cast von Server  
2 Client rechnet herunterzuladende vmdk und/oder deltas aus + basex_y-2.vmdk.asc  
3 Client lädt asc, vmdk und/oder delta herunter, prüft jeden Download mit Prüfsumme  
4 Update durchführen mit rdiff  

Üblicherweise ist beim ersten Start der PrivacyMachine keine BaseDisk.vmdk lokal vorhanden, d.h. es
muss auf jeden Fall ein Update durchgeführt werden. Mögliche Implementierung: PM nimmt anfangs eine
BaseDisk Version "0.0.0.0" an, damit wird dann "1.0.0.0" als update vorgeschlagen.

### Prüfsummen
Für jede heruntergeladene Datei werden folgende Prüfsumme erstellt und heruntergeladen zwecks
verifikation:
* SHA256 im app cast (Liste mit Dateinamen und zugehörigen Prüfsummen) => nur ein HTTP request für
alle Dateien, berechnet mit QCryptographicHash [1]
* GPG-Signatur in asc-Datei


[1] SHA256 erzeugt zu Testzwecken mittels:  
  
    openssl dgst -sha256 -hex <Dateiname>  

### Update-Intervall
Core-Team hat aktuell (20160912) keine 24h-Hotline 
=> best Case: Ein Update jeden Tag
=> PM pollt 1x pro Tag nach Updates

#### Literatur
[1] https://stackoverflow.com/questions/2077550/how-can-i-enable-auto-updates-in-a-qt-cross-platform-application  
[2] https://github.com/pypt/fervor/tree/autoupdate#description  

## Bisher erfolglose/verworfende Versuche ##
o) User booten pro VM-Maske eine LiveCD die am Server erzeugt/aktualisiert wird
  -> Problem: Der Aktualisierungsprozess der das ISO auf Basis ubuntu 14.04 baut wurde im märz 2015 aufgrund von Paketänderungen instabil.
o) base.vdi per zsync verteilen.
  -> Problem: Damit der Client erkennt welche Änderungen er herunter laden muss, muss er das ganze File (1.4GB) selbst komprimieren -> Dauert 10 Minuten am Client

## Test von rdiff vs xdelta:
xdelta failed komplett und erzeugt 3.1G delta file!
rdiff produziert 270MB delta file:

generate signature file (muss nicht mitausgeliefert werden):
     rdiff signature 2016_07_06_23:58/BaseDisk.vdi BaseDisk_2016_07_06.signature 

generate delta file:
     rdiff delta BaseDisk_2016_07_06.signature 2016_07_10_15:51/BaseDisk.vdi BaseDisk_2016_07_06_to_2016_07_11.delta

delta anwenden: 
     rdiff patch 2016_07_06_23:58/BaseDisk.vdi BaseDisk_2016_07_06_to_2016_07_11.delta new_BaseDisk__2016_07_11.vdi

Filegrößen:

3.2G    2016_07_06_23:58/BaseDisk.vdi
3.1G    2016_07_10_15:51/BaseDisk.vdi

19M     BaseDisk_2016_07_06.signature
270M    BaseDisk_2016_07_06_to_2016_07_11.delta

### Was ist zum Updaten:
BaseDisk.vdi:
  Wenn neue Browser, openvpn, ...
Binaries - oft:
  PrivacyMachine.exe
Binaries - selten:
  Qt-Libs
  FreeRDP/RemoteDisplay
??:
VPN-Configs
Firefox-Profil + Addons

Was wenn PrivacyMachine.exe und BaseDisk.vdi auseinanderlaufen? -> Versionierung!

http://www.vtk.org/Wiki/CmakeMingw
https://ci.freerdp.com/

## CI/Deployment

Linux-Builds:
 1. Variante: Generisches Linux-Package: http://appimage.org/
 2. Variante: DEB-Packete bauen: 2x debian + 2 x ubuntu
       http://jenkins-debian-glue.org/
       https://build.opensuse.org/ Windows via crosscompile?            
  Hosting von packages:
    https://packagecloud.io/#home  
    https://bintray.com/

Windows-Install-Update:
 1. Variante: Portable
 2. Variante: WIX Installer http://stackoverflow.com/questions/471424/wix-tricks-and-tips
        https://www.firegiant.com/wix/tutorial/upgrades-and-modularization/
 3. Variante: Nsis (alt und schimmlig, cross-build möglich)
 
