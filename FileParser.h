/*******************************************************************
This file is part of OpticsBenchUI.

OpticsBenchUI is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

#ifndef FILEPROPERTIES_H
#define FILEPROPERTIES_H

#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QRegExp>
#include "QsLog.h"

class FileParser
{
   public:
   FileParser(QObject *parent = 0);
   ~FileParser();

   void setDir(QString _dir);
   void setPrefix(QString _prefix);
   void setSuffix(QString _suffix);
   void setDel(QString _del);
   void setOrder(QString _order);
   void setType(QString _type);
   void setFormat(QString _format);
   void setSep(QString _sep);
   void setDelay(QString _delay);
   void setNameList(QString _nameList);


   QString getDir();
   QString getPrefix();
   QString getSuffix();
   QString getDel();
   QString getOrder();
   QString getType();
   QString getFormat();
   QString getSep();
   QString getDelay();
   QString getParsedFileName();
   QVector<QString> getNameList();

   bool getFileData();

   double* getArray();
   int     getArrayHeight();
   int     getArrayWidth();
   QVector<double> getDataList();

   private:
    QString dir;
    QString prefix;
    QString suffix;
    QString del;
    QString order;
    QString format;
    QString sep;
    QString delay;
    QString type;
    QVector<QString> nameList;
    QString parsedFileName; 
    double  *array;  
    int arrayHeight, arrayWidth; 
    QVector<double> dataList;
  
};
#endif
