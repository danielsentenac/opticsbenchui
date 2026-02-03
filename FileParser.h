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

#ifndef FILEPARSER_H
#define FILEPARSER_H

#include <unistd.h>

#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include "QsLog.h"

/// \ingroup utils
/// Parses data files and extracts arrays and metadata.
class FileParser
{
   public:
   /// Constructor.
   /// \param parent Parent object.
   FileParser(QObject *parent = 0);
   /// Destructor.
   ~FileParser();

   /// Set the directory containing input files.
   /// \param _dir Directory path.
   void setDir(QString _dir);
   /// Set the filename prefix.
   /// \param _prefix Prefix string.
   void setPrefix(QString _prefix);
   /// Set the filename suffix.
   /// \param _suffix Suffix string.
   void setSuffix(QString _suffix);
   /// Set the delimiter value.
   /// \param _del Delimiter string.
   void setDel(QString _del);
   /// Set the filename ordering scheme.
   /// \param _order Ordering string.
   void setOrder(QString _order);
   /// Set the file type identifier.
   /// \param _type Type string.
   void setType(QString _type);
   /// Set the format string.
   /// \param _format Format string.
   void setFormat(QString _format);
   /// Set the separator used in the file.
   /// \param _sep Separator string.
   void setSep(QString _sep);
   /// Set the delay value used for sequencing.
   /// \param _delay Delay string.
   void setDelay(QString _delay);
   /// Set a list of filenames.
   /// \param _nameList Name list string.
   void setNameList(QString _nameList);


   /// Return the directory.
   QString getDir();
   /// Return the filename prefix.
   QString getPrefix();
   /// Return the filename suffix.
   QString getSuffix();
   /// Return the delimiter value.
   QString getDel();
   /// Return the order.
   QString getOrder();
   /// Return the type.
   QString getType();
   /// Return the format string.
   QString getFormat();
   /// Return the separator.
   QString getSep();
   /// Return the delay value.
   QString getDelay();
   /// Return the computed filename.
   QString getParsedFileName();
   /// Return the list of filenames.
   QVector<QString> getNameList();

   /// Load and parse file data into internal structures.
   bool getFileData();

   /// Return the parsed data array.
   double* getArray();
   /// Return the parsed array height.
   int     getArrayHeight();
   /// Return the parsed array width.
   int     getArrayWidth();
   /// Return the parsed data as a list.
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
