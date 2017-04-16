/*==============================================================================
        Copyright (c) 2013-2016 by the Developers of PrivacyMachine.eu
                         contact@privacymachine.eu
     OpenPGP-Fingerprint: 0C93 F15A 0ECA D404 413B 5B34 C6DE E513 0119 B175

                     Licensed under the EUPL, Version 1.1
     European Commission - subsequent versions of the EUPL (the "Licence");
        You may not use this work except in compliance with the Licence.
                  You may obtain a copy of the Licence at:
                        http://ec.europa.eu/idabc/eupl

 Unless required by applicable law or agreed to in writing, software distributed
              under the Licence is distributed on an "AS IS" basis,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
      See the Licence for the specific language governing permissions and
                        limitations under the Licence.
==============================================================================*/

#pragma once

#include <QObject>
#include <QtTest>
#include <QtNetwork>

#include "deprecated_fvupdater.h"

#include "utils.h"

/// Tests the patching / download of base disks using fervor.
///
/// TestPatching needs `file://localhost/media/pmTests/` accessible on the current machine.
/// * On Linux, as superuser, create a link to `pm/tests`:
///     mkdir /media
///     cd /media
///     ln -s <path to pm/tests> pmTests
/// * On Windows, create a share called "media", pointing to the pm folder
// XXX AL change from media to mnt, as media relates to physical media that are mounted, rather than arbitrary folders
// being mounted there


class TestPatching : public QObject
{
    Q_OBJECT
  public:
//    explicit TestPatching(QObject *parent = NULL)
//    { }

  // Amongst the private slots, test functions are named test<test case>(). This helps in distinguishing them from the
  // common QTest slots that are called by QTestLib, namely 
  // * initTestCase() before first test function
  // * cleanupTestCase() after last test function
  // * init() before each test function
  // * cleanup() after each test function
  private slots:

    void initTestCase()
    { }

    void cleanupTestCase()
    { }

    void init()
    {
      updateInstalled_ = false;
      
    }

    void cleanup()
    { }

    
    void testVerifyUpdate_7z();

    void testVerifyUpdate();

    void testFalsifyObsolesce();

  signals:

  public slots:
    void slotUpdateInstalled()
    {
      updateInstalled_ = true;
    }

  private:

    bool addFileToZip(
      QuaZip* zipFile, QString inputFilePath, QString resultingFileName, QString resultingRelativeAddtitionalPath);

    bool checksumsMatch( const QString & filePath1, const QString & filePath2 ); 

    bool findUpdate();

    bool download( QString url, QString localFileName, ulong milliseconds = 500U );

    /// Triggers an update and attempts to wait until the update has been installed. Aborts after some timeout if the 
    /// signalUpdateInstalled is not received.
    bool installUpdate();
    
    bool updateInstalled_;

};

