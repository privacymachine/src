#ifndef VMMASKFIREFOXCONFIG_H
#define VMMASKFIREFOXCONFIG_H

#include <QList>
#include <QPair>
#include <QVariant>
#include <QString>



class VmMaskFireFoxConfig
{
  public:
    VmMaskFireFoxConfig();

  private:
    static QList< QPair<QString, QList<QVariant> > > configValueList_;
};

#endif // VMMASKFIREFOXCONFIG_H
