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

#include "XmlUpdateParser.h"
#include "utils.h"

XmlUpdateParser::XmlUpdateParser()
{
}

bool XmlUpdateParser::parse(QByteArray parRawData)
{
  QDomDocument dom;
  QString errorMsg;
  int errorLine;
  int errorColumn;
  if (!dom.setContent(parRawData, &errorMsg, &errorLine, &errorColumn))
  {
    IWARN("XML-parsing error: line:" + QString::number(errorLine) + ", column:" + QString::number(errorColumn));
    return false;
  }

  QDomElement root = dom.documentElement();
  if (root.tagName() != "rss")
  {
    IWARN("Unexpected XML-Document");
    return false;
  }

  // get <channel>
  QDomNode nodeChannel = root.firstChild();
  if (nodeChannel.nodeName() != "channel")
  {
    IWARN("Unexpected XML-Document");
    return false;
  }

  // loop over all <item> (which is as Blog-Entry)
  QDomNode childOfChannel = nodeChannel.firstChild();
  while (!childOfChannel.isNull())
  {
    if (childOfChannel.nodeName() == "item")
    {
      QString title;
      QDateTime date;
      QString description;
      QString updateType;

      QDomNode nodeTitle = childOfChannel.firstChildElement("title");
      if (!nodeTitle.isNull())
        title = nodeTitle.toElement().text();

      QDomNode nodeDate = childOfChannel.firstChildElement("pubDate");
      if (!nodeDate.isNull())
      {
        QString dateStr = nodeDate.toElement().text();
        QString format = "ddd, dd MMM yyyy HH:mm:ss";
        if( dateStr.size() > 5)
        {
          date = QDateTime::fromString(dateStr.left(dateStr.size()-4),format);
        }
        else
        {
          IERR("Could not parse date: "+ dateStr+"  INVALID FORMAT");
        }
        if( !date.isValid() )
          IERR("Could not parse date: "+ dateStr.left(dateStr.size()-4)+"  INVALID DATE");
      }
      QDomNode nodeDesc = childOfChannel.firstChildElement("description");
      if (!nodeDesc.isNull())
        description = nodeDesc.toElement().text();

      QDomNode nodeType = childOfChannel.firstChildElement("PmUpdateType");
      if (!nodeType.isNull())
        updateType = nodeType.toElement().text();

      QString versionStr;
      QDomNode nodeVersion = childOfChannel.firstChildElement("PmVersion");
      if (!nodeVersion.isNull())
        versionStr = nodeVersion.toElement().text();

      if (updateType == "Binary")
      {
        UpdateInfoBinary newBinary;
        newBinary.Title = title;
        newBinary.Date = date;
        newBinary.Description = description;
        newBinary.Date = date;
        newBinary.Version.parse(versionStr); // ignore errors?

        // !! QCryptographicHash::Sha3_256 does NOT implement sha3_256 !!
        // https://bugreports.qt.io/browse/QTBUG-59770?jql=text%20~%20%22QCryptographicHash%22
        // So we use sha256 instead till qt5.9 is avaiable in debian and its distributions
        // QDomNode nodeCheckSums = childOfChannel.firstChildElement("PmSHA3-256CheckSums");
        QDomNode nodeCheckSums = childOfChannel.firstChildElement("PmSHA256CheckSums");
        if (!nodeCheckSums.isNull())
        {
          QDomNode nodeCheckSum = nodeCheckSums.firstChild();
          while(!nodeCheckSum.isNull() && nodeCheckSum.nodeName() == "CheckSum")
          {
            QString os;
            if (nodeCheckSum.attributes().contains("os"))
              os = nodeCheckSum.attributes().namedItem("os").nodeValue();

            QString url;
            if (nodeCheckSum.attributes().contains("url"))
              url = nodeCheckSum.attributes().namedItem("url").nodeValue();

            QString checkSum = nodeCheckSum.toElement().text();

            // append the CheckSum
            CheckSumListBinary newCheckSum;
            newCheckSum.Os = os;
            newCheckSum.Url = url;
            newCheckSum.CheckSum = checkSum;
            newBinary.CheckSums.append(newCheckSum);

            // find next <CheckSum>
            nodeCheckSum = nodeCheckSum.nextSibling();
          }
        }

        // store the data
        binaries_.append(newBinary);
      }

      if(updateType == "Config")
      {
        UpdateInfoConfig newConfig;
        newConfig.Title = title;
        newConfig.Date = date;
        newConfig.Description = description;
        newConfig.Date = date;
        newConfig.Version.parse(versionStr); // ignore errors?

        // !! QCryptographicHash::Sha3_256 does NOT implement sha3_256 !!
        // https://bugreports.qt.io/browse/QTBUG-59770?jql=text%20~%20%22QCryptographicHash%22
        // So we use sha256 instead till qt5.9 is avaiable in debian and its distributions
        // QDomNode nodeCheckSums = childOfChannel.firstChildElement("PmSHA3-256CheckSums");
        QDomNode nodeCheckSums = childOfChannel.firstChildElement("PmSHA256CheckSums");
        if (!nodeCheckSums.isNull())
        {
          QDomNode nodeCheckSum = nodeCheckSums.firstChild();
          while(!nodeCheckSum.isNull() && nodeCheckSum.nodeName() == "CheckSum")
          {
            QString os;
            if (nodeCheckSum.attributes().contains("os"))
              os = nodeCheckSum.attributes().namedItem("os").nodeValue();

            QString url;
            if (nodeCheckSum.attributes().contains("url"))
              url = nodeCheckSum.attributes().namedItem("url").nodeValue();

            QString checkSum = nodeCheckSum.toElement().text();

            // append the CheckSum
            CheckSumListConfig newCheckSum;
            newCheckSum.Os = os;
            newCheckSum.Url = url;
            newCheckSum.CheckSum = checkSum;
            newConfig.CheckSums.append(newCheckSum);

            // find next <CheckSum>
            nodeCheckSum = nodeCheckSum.nextSibling();
          }
        }

        // store the data
        configs_.append(newConfig);
      }

      if(updateType == "BaseDisk")
      {
        UpdateInfoBaseDisk newBaseDisk;
        newBaseDisk.Title = title;
        newBaseDisk.Date = date;
        newBaseDisk.Description = description;
        newBaseDisk.Date = date;
        newBaseDisk.Version.parse(versionStr); // ignore errors?

        // !! QCryptographicHash::Sha3_256 does NOT implement sha3_256 !!
        // https://bugreports.qt.io/browse/QTBUG-59770?jql=text%20~%20%22QCryptographicHash%22
        // So we use sha256 instead till qt5.9 is avaiable in debian and its distributions
        // QDomNode nodeCheckSums = childOfChannel.firstChildElement("PmSHA3-256CheckSums");
        QDomNode nodeCheckSums = childOfChannel.firstChildElement("PmSHA256CheckSums");
        if (!nodeCheckSums.isNull())
        {
          QDomNode nodeCheckSum = nodeCheckSums.firstChild();
          while(!nodeCheckSum.isNull() && nodeCheckSum.nodeName() == "CheckSum")
          {
            QString url;
            if (nodeCheckSum.attributes().contains("url"))
              url = nodeCheckSum.attributes().namedItem("url").nodeValue();

            QString upFrom;
            if (nodeCheckSum.attributes().contains("from"))
              upFrom = nodeCheckSum.attributes().namedItem("from").nodeValue();

            QString checkSum = nodeCheckSum.toElement().text();

            // append the CheckSum
            CheckSumListBaseDisk newCheckSum;
            newCheckSum.ComponentMajorUp = upFrom.toInt();
            newCheckSum.Url = url;
            newCheckSum.CheckSum = checkSum;
            newBaseDisk.CheckSums.append(newCheckSum);

            // find next <CheckSum>
            nodeCheckSum = nodeCheckSum.nextSibling();
          }
        }

        // store the data
        baseDisks_.append(newBaseDisk);
      }

    }
    // find next
    childOfChannel = childOfChannel.nextSibling();
  }

  return true;
}
