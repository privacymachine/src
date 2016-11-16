#pragma once

#include <QObject>
#include <QtTest>
#include <QtNetwork>

#include "fvupdater.h"

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
//    explicit TestPatching(QObject *parent = 0) 
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

