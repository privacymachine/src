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
      ConfigVPN* vpnConfig = new ConfigVPN();
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
