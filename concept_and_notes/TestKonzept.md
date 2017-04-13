# Literatur
<http://doc.qt.io/qt-5/qttest-index.html>
<https://wiki.qt.io/Writing_Unit_Tests>

# Ausführen der Tests
Die mit QTestLib erstellten Tests können entweder direkt mittels `pm/build/testAll.sh` oder über ctest (von cmake) gestartet werden.

# Ordnerstruktur
Was ist an der Directory/Build-Struktur anders?
* Alles was unter `pm` lag ist jetzt eine statische Library `libpm` welche von der PrivacyMachine.exe und den Testern gelinkt wird.  
* Das Verzeichnis wo alles ausgeführt wird ist `<gitroot>/pm/build/`  
* ./testAll.sh startet alle UnitTests.  
* um einen einzelnen Test selbst zu starten:  
  `cd build`  
  `../tests/GuiTest`  
  oder  
  `../tests/GuiTest -v2 -vs`  
