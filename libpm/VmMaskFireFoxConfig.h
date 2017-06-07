#ifndef VMMASKFIREFOXCONFIG_H
#define VMMASKFIREFOXCONFIG_H

#include <QList>
#include <QPair>
#include <QVariant>
#include <QString>
#include "VmMaskUserConfig.h"
#include "VmMaskStaticConfig.h"
#include "VmMaskCurrentConfig.h"

class VmMaskFireFoxConfig
{
  public:
    VmMaskFireFoxConfig();

    /// \brief getConfigValueList
    /// \return when diceNewVmMaskFireFoxConfig was called a list of configuration parameters otherwise a empty list
    QList< QPair<QString, QVariant> > getConfigValueList() { return configValueList_; }

    /// \brief getPrefs
    /// \return when diceNewVmMaskFireFoxConfig was called a firefox prefs.js otherwise a empty string
    QString getPrefs();


    friend VmMaskCurrentConfig * VmMaskUserConfig::diceNewVmMaskConfig(VmMaskStaticConfig *);

  protected:

    /// \brief diceNewVmMaskFireFoxConfig
    /// \brief generates a randomized firefox configuration
    /// \brief sould only be called by VmMaskUserConfig::diceNewVmMaskConfig
    void diceNewVmMaskFireFoxConfig();

  private:

    /// \brief variableConfigValueList_
    /// \brief static list that holds the preferences to set always
    static QList< QPair<QString, QList<QVariant> > > variableConfigValueList_;

    /// \brief staticConfigValueList_
    /// \brief static list that holds the preferences to randomize
    static QList< QPair<QString, QVariant> > staticConfigValueList_;

    /// \brief configValueList_
    /// \brief list to store the current preferences
    QList< QPair<QString, QVariant> > configValueList_;
};

#endif // VMMASKFIREFOXCONFIG_H
