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
    /// \return a empty list or when diceNewVmMaskFireFoxConfig was already called a list of configuration parameters
    QList< QPair<QString, QVariant> > getConfigValueList() { return configValueList_; }

    QString getPrefs();

    friend VmMaskCurrentConfig * VmMaskUserConfig::diceNewVmMaskConfig(VmMaskStaticConfig *);


  //protected:

    /// \brief diceNewVmMaskFireFoxConfig
    /// \brief generates a randomized firefox configuration
    /// \brief sould only be called by VmMaskUserConfig::diceNewVmMaskConfig
    void diceNewVmMaskFireFoxConfig();

  private:
    static QList< QPair<QString, QList<QVariant> > > variableConfigValueList_;
    static QList< QPair<QString, QVariant> > staticConfigValueList_;

    QList< QPair<QString, QVariant> > configValueList_;
};

#endif // VMMASKFIREFOXCONFIG_H
