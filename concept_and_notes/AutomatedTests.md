# References
<http://doc.qt.io/qt-5/qttest-index.html>
<https://wiki.qt.io/Writing_Unit_Tests>

# Execution of tests
The with QTestLib created tests can be started directly via 'pm/build/testAll.sh' or over ctest (cmake-binary).

* ./testAll.sh starts all tests
* to manual start a single test:
  'cd working_dir'  
  './GuiTest'  
  or  
  './GuiTest -v2 -vs'  
