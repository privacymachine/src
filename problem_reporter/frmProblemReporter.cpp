/*==============================================================================
        Copyright (c) 2013-2017 by the Developers of PrivacyMachine.eu
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

#include "frmProblemReporter.h"
#include "ui_frmProblemReporter.h"

#define QUAZIP_STATIC
#include <quazipfile.h>
#include <quazipfileinfo.h>

#include <QLayout>
#include <QFileDialog>
#include <QProcess>
#include <QStringList>
#include <QDirIterator>

#include <QMessageBox>

#include "../libpm/utils.h"

#include <iostream>
using namespace std;

#ifdef WIN32
  #include <Windows.h>
#endif

extern QString staticGlobalLogFileName; // defined in main.cpp

frmProblemReporter::frmProblemReporter(QString executableDir, QWidget *parent) :
  executableAbsDir_(executableDir),
  QMainWindow(parent),
  ui(new Ui::frmProblemReporter)
{
  ui->setupUi(this);

  setWindowTitle("ProblemReporter of PrivacyMachine " + QString(constPrivacyMachineVersion));
  setWindowIcon(QIcon(":/images/PrivacyMachine_Logo_Icon32.png"));

  connect(ui->txtProblemDescription, SIGNAL(textChanged()), this, SLOT(txtProblemDescription_textChanged()));
  connect(ui->btnFolderSelect, SIGNAL(clicked()), this, SLOT(btnFolderSelect_Clicked()));
  connect(ui->btnStartCreation, SIGNAL(clicked()), this, SLOT(btnStartCreation_Clicked()));

  saveFolder_ = "";
  ui->btnFolderSelect->setEnabled(true);

  checkIfAllDataAvailable();
}

frmProblemReporter::~frmProblemReporter()
{
  delete ui;
}

void frmProblemReporter::btnFolderSelect_Clicked()
{
  QFileDialog dlg(this);
  dlg.setFileMode(QFileDialog::Directory);
  dlg.setOption(QFileDialog::ShowDirsOnly, true);
  dlg.setViewMode(QFileDialog::Detail);

  QString folderName = dlg.getExistingDirectory(this, "Select Folder in which to save the ProblemReport-ZIP-File");
  if (folderName.length())
  {
    if (!folderName.endsWith("/") || folderName.endsWith("\\"))
      folderName += "/";

    if (RunningOnWindows())
      folderName.replace("/","\\");
  }
  saveFolder_ = folderName; // is empty if dialog was canceled;
  ui->txtOutputFolder->setText(saveFolder_);

  checkIfAllDataAvailable();
}

void frmProblemReporter::checkIfAllDataAvailable()
{
  if (ui->txtProblemDescription->toPlainText().size() > 0 && saveFolder_.size() > 0)
    ui->btnStartCreation->setEnabled(true);
  else
    ui->btnStartCreation->setEnabled(false);

  if (saveFolder_.size())
    ui->statusBar->showMessage("Outputfolder will be: " + saveFolder_);
  else
    ui->statusBar->showMessage("Outputfolder is not selected yet");
}

void frmProblemReporter::txtProblemDescription_textChanged()
{
  checkIfAllDataAvailable();
}

void frmProblemReporter::btnStartCreation_Clicked()
{
  QApplication::setOverrideCursor(Qt::ArrowCursor);
  this->setCursor(Qt::ArrowCursor);
  ui->txtProblemDescription->setCursor(Qt::ArrowCursor);
  QApplication::processEvents();

  bool bOk = true;

  QString zipFilePath;
  zipFilePath = saveFolder_ + "PrivacyMachine_ProblemReport" + currentTimeStampAsISOFileName() + ".zip";

  QuaZip zipFile(zipFilePath);
  zipFile.setFileNameCodec("IBM866");
  if (!zipFile.open(QuaZip::mdCreate))
  {
    IERR("error from zip.open(): " + QString::number(zipFile.getZipError()));
    bOk = false;
  }

  if (bOk) if(!addErrorDescription(&zipFile, ui->txtProblemDescription->toPlainText()))
    bOk = false;

  if (bOk) if(!addAllPMLogsToZipFile(&zipFile))
    bOk = false;

  // At the end add the current logfile
  if (zipFile.isOpen())
    if (!addFileToZip(&zipFile, staticGlobalLogFileName, "PrivacyMachine_ProblemReporter.txt", "")) bOk = false;

  zipFile.close();
  if (zipFile.getZipError() != 0)
  {
    IERR("error on zipfile.close(): " + QString::number(zipFile.getZipError()));
    bOk = false;
  }

  if (bOk)
  {
    QString msg = "Successfully created the ProblemReport-ZIP-File: " + zipFilePath;
    QMessageBox::information(this, "Creation succeeded", msg);
    ui->statusBar->showMessage(msg);
  }
  else
  {
    QMessageBox::warning(this, "Creation failed", "Creation of the ProblemReport-ZIP-File '" + zipFilePath + "' didn't completely succeeded. \nPlease check the logfile at: " + staticGlobalLogFileName);
    ui->statusBar->showMessage("Creation failed");
  }
}

bool frmProblemReporter::addFileToZip(QuaZip* zipFile, QString inputFilePath, QString resultingFileName, QString resultingRelativeAddtitionalPath)
{
  QFileInfo fileInfo(inputFilePath);
  if (!fileInfo.isFile())
  {
    // It's ok to add non-existing files to this function (because it makes the caller easier readable)
    ILOG("file " + inputFilePath + " does not exist.");
    return true;
  }

  QFile inFile;
  QuaZipFile outFile(zipFile);

  inFile.setFileName(fileInfo.filePath());
  if (!inFile.open(QIODevice::ReadOnly))
  {
    IERR("error from inFile.open(): " + inFile.errorString());
    return false;
  }

  if (!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(resultingRelativeAddtitionalPath + resultingFileName, inputFilePath)))
  {
    IERR("error adding file: " + inputFilePath + ": " + outFile.getZipError());
    return false;
  }

  // copy the content
  char c;
  while (inFile.getChar(&c) && outFile.putChar(c));

  if (outFile.getZipError() != UNZ_OK)
  {
    IERR("error calling putChar(): " + outFile.getZipError());
    return false;
  }

  outFile.close();

  if (outFile.getZipError() != UNZ_OK)
  {
    IERR("error from close(): " + outFile.getZipError());
    return false;
  }

  return true;
}

bool frmProblemReporter::addErrorDescription(QuaZip* zipFile, QString errorDescription)
{
  QuaZipFile outFile(zipFile);

  QuaZipNewInfo newFile("ProblemDescription.txt");
  newFile.setPermissions(QFile::ReadUser | QFile::ReadOwner | QFile::ReadGroup | QFile::ReadOther);
  if (!outFile.open(QIODevice::WriteOnly, newFile))
  {
    IERR("error adding file: ProblemDescription.txt: " + outFile.getZipError());
    return false;
  }

  outFile.write(qPrintable(errorDescription));

  if (outFile.getZipError() != UNZ_OK)
  {
    IERR("error calling putChar(): " + outFile.getZipError());
    return false;
  }

  outFile.close();

  if (outFile.getZipError() != UNZ_OK)
  {
    IERR("error from close(): " + outFile.getZipError());
    return false;
  }

  return true;
}

bool frmProblemReporter::addAllPMLogsToZipFile(QuaZip* zipFile)
{

  // TODO: Ugly fix is now in PmData
  QDir userConfigDir = QString::fromStdString(getPmDefaultConfigQDir());

  if (!userConfigDir.exists())
  {
    IERR("error getting user config dir");
    return false;
  }

  QString logfilePrefix = userConfigDir.path() + "/logs";
  QDirIterator it(logfilePrefix, QDirIterator::NoIteratorFlags);
  while (it.hasNext())
  {
    QString filePath = it.next();
    QFile ff(filePath);
    QFileInfo fileInfo(ff);


    if ( filePath.contains("Log") && fileInfo.exists() && fileInfo.isFile() )
      if (!addFileToZip(zipFile, filePath, fileInfo.fileName(), "logs/")) return false;
  }
  return true;
}
