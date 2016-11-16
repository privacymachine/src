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

#include <QMainWindow>

#include <QProcess>

class QuaZip;

namespace Ui {
  class frmProblemReporter;
}

class frmProblemReporter : public QMainWindow
{
    Q_OBJECT

  public:
    explicit frmProblemReporter(QString executableAbsDir, QWidget *parent = 0);
    ~frmProblemReporter();

  private slots:
    void btnStartCreation_Clicked();
    void btnFolderSelect_Clicked();
    void txtProblemDescription_textChanged();

  private:
    void checkIfAllDataAvailable();
    bool addAllPMLogsToZipFile(QuaZip* zipFile);
    bool addFileToZip(QuaZip* zipFile, QString inputFilePath, QString resultingFileName, QString resultingRelativeAddtitionalPath);
    bool addErrorDescription(QuaZip* zipFile, QString errorDescription);

    QString saveFolder_;
    QProcess proc_;
    QString executableAbsDir_;
    Ui::frmProblemReporter *ui;
};

