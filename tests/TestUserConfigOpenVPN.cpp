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

#include <QtTest>

#include "../libpm/UserConfig.h"

class TestUserConfigOpenVPN : public QObject
{
    Q_OBJECT
  public:
    explicit TestUserConfigOpenVPN(QObject *parent = 0) {}

  private slots:

    void initTestCase() // called from QTestLib before all tests
    {
      currentPath_ = QDir::currentPath();
      pmConfig_ = new UserConfig(currentPath_ + "/conf/PrivacyMachine_Example_en.ini", currentPath_, currentPath_);
    }

    void openIniFile()
    {
      QVERIFY(pmConfig_->readFromFile() == true);
    }

    void checkNumberOfConfiguredVPNs()
    {
      QVERIFY2(pmConfig_->getConfiguredVPNs().count() > 0, "seems nobody has configured a VPN!");
    }

    void testOpenVpnConfigFiles()
    {
      VpnConfig* vpnConfig = new VpnConfig();
      QString configPath;
      QString configFilter;
      QString directoryName;

      pmConfig_->parseOpenVpnConfigFiles("C:\\some\\path\\provider.*", configPath, directoryName, configFilter);
      QVERIFY(configPath == "C:\\some\\path");
      QVERIFY(configFilter == "provider.*");

      pmConfig_->parseOpenVpnConfigFiles("/some/path/*.ovpn", configPath, directoryName, configFilter);
      QVERIFY(configPath == "/some/path");
      QVERIFY(configFilter == "*.ovpn");

      pmConfig_->parseOpenVpnConfigFiles("/some/path/provider.*", configPath, directoryName, configFilter);
      QVERIFY(configPath == "/some/path");
      QVERIFY(configFilter == "provider.*");

      pmConfig_->parseOpenVpnConfigFiles("{INSTALL_DIR}/vpn/*.conf", configPath, directoryName, configFilter);
      QVERIFY(configPath == QString(currentPath_ + "/vpn"));
      QVERIFY(configFilter == "*.conf");

      pmConfig_->parseOpenVpnConfigFiles("{USER_CONFIG_DIR}/vpn/germany.conf", configPath, directoryName, configFilter);
      QVERIFY(configPath == QString(currentPath_ + "/vpn"));
      QVERIFY(configFilter == "germany.conf");

    }

    void cleanupTestCase() // called from QTestLib after all tests
    {
      delete (pmConfig_);
    }

  signals:

  public slots:

  private:
    UserConfig* pmConfig_;
    QString currentPath_;
};

QTEST_MAIN(TestUserConfigOpenVPN)
#include "TestUserConfigOpenVPN.moc"
