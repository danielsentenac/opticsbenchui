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

#include "FileParser.h"

FileParser::FileParser(QObject *parent)
{
 dir = "";
 prefix = "";
 suffix = "";
 del = "";
 sep = "";
 format = "";
 type = "";
 order = "";
 parsedFileName = "";
 array = NULL;
}

FileParser::~FileParser()
{
}

void
FileParser::setDir(QString _dir)
{
  dir = _dir;
}
void
FileParser::setPrefix(QString _prefix)
{
  prefix = _prefix;
}
void
FileParser::setSuffix(QString _suffix)
{
  suffix = _suffix;
}
void
FileParser::setDel(QString _del)
{
  del = _del;
}
void
FileParser::setOrder(QString _order)
{
  order = _order;
}
void
FileParser::setType(QString _type)
{
  QLOG_DEBUG() << "FileParser::setType> _type : " <<  _type;
  QString list = "";
  list = _type.mid(_type.indexOf("[") + 1, _type.indexOf("]") - _type.indexOf("[") - 1);
  QLOG_DEBUG() << "FileParser::setType> list : " <<  list;
  if (!list.isNull()) { 
    list.replace("_"," ");
    QLOG_INFO() << "FileParser::setType> list : " <<  list;
    setNameList(list);
    type = _type.left(_type.indexOf("["));
    QLOG_DEBUG() << "FileParser::setType> type : " <<  type;
  }
  else
   type = _type;
}
void
FileParser::setFormat(QString _format)
{
  format = _format;
}
void
FileParser::setSep(QString _sep)
{
  if ( _sep == "TAB" )
   sep = "\t";
  else if ( _sep == "SPACE" )
   sep = " ";
  else if ( _sep == "COMMA" )
   sep = ",";

}
void
FileParser::setDelay(QString _delay)
{
  delay = _delay;
}

void
FileParser::setNameList(QString _nameList)
{
  QStringList nameRefList = _nameList.split(",");
  foreach (const QString &nameRef, nameRefList) { 
    QLOG_INFO() << "FileParser::setNameList> parameter : " <<  nameRef;
    nameList.push_back(nameRef);
  }
}

QString
FileParser::getDir()
{
  return dir;
}
QString
FileParser::getPrefix()
{
  return prefix;
}
QString
FileParser::getSuffix()
{
  return suffix;
}
QString
FileParser::getDel()
{
  return del;
}
QString
FileParser::getOrder()
{
  return order;
}
QString
FileParser::getType()
{
  return type;
}
QString
FileParser::getFormat()
{
  return format;
}
QString
FileParser::getSep()
{
  return sep;
}
QString
FileParser::getDelay()
{
  return delay;
}
QString
FileParser::getParsedFileName()
{
  return parsedFileName;
}
QVector<QString>
FileParser::getNameList()
{
  return nameList;
}

bool
FileParser::getFileData()
{
  bool success = false;
  // Create QDir object to navigate into directory path
  QDir directory(dir);
  QDir subdirectory(dir + "/Results");
  QStringList fileList = directory.entryList(QDir::Files, QDir::Time | QDir::Reversed);
  QStringList subfileList = subdirectory.entryList(QDir::Files, QDir::Time | QDir::Reversed);
  // Check directory and Results subdirectory content deletion
  if ( del == "BEFORE" ) {
    int cnt = 0;
    foreach (const QString &filename, fileList) {
      if ( cnt == fileList.count() )
        break;
      QLOG_DEBUG() << "FileParser::getFileData> deleting file" << filename;
      directory.remove( filename );
      cnt++;
    }
    cnt = 0;
    foreach (const QString &filename, subfileList) {
      if ( cnt == subfileList.count() )
        break;
      QLOG_DEBUG() << "FileParser::getFileData> deleting file" << filename;
      subdirectory.remove( filename );
      cnt++;
    }
  }

  // Apply a delay before new dir scan
  usleep( (int) (delay.toFloat() * 1000000) );

  // Look for the requested file from new dir scan
  QDir freshdir(dir + "/Results");
  QFileInfoList freshFileInfoList = freshdir.entryInfoList(QDir::NoFilter, QDir::Time | QDir::Reversed);
  QFile file ("");
  uint filetime = 0;
  parsedFileName = "";
  foreach (const QFileInfo &fileinfo, freshFileInfoList) {
     QString filename = fileinfo.fileName();
     QLOG_DEBUG() << "FileParser::getFileData> treating file" << filename;
     QDateTime date (fileinfo.lastModified());
     uint filetimeTmp = static_cast<uint>(date.toSecsSinceEpoch());
     if ( filename.left(prefix.length()) == prefix && 
          filename.right(suffix.length()) == suffix ) {
        if ( order == "FIRST" ) {
         file.setFileName(freshdir.filePath(filename));
         break;
        }
        else if ( order == "LAST" && filetime < filetimeTmp ) {
           file.setFileName(freshdir.filePath(filename));
           filetime = filetimeTmp;         
        }
     }
  }
  // parse the file data
  if ( file.fileName() == "" ) {
    QLOG_WARN () << "FileParser::getFileData> No available file " 
                 << " with prefix " << prefix
                 << " suffix " << suffix
                 << " in directory " << dir + "/Results";
     return false;
  }
  else {
    QLOG_INFO () << "FileParser::getFileData> parsing file " << file.fileName();
    parsedFileName = file.fileName();
    // Open File 
    if ( format == "ASCII" )	
      file.open(QIODevice::ReadOnly | QIODevice::Text);
    else
      file.open(QIODevice::ReadOnly); 
    QTextStream in(&file);
    if ( type == "ARRAY" ) {
      // Get array size in File
      arrayHeight = 1;
      arrayWidth = 0;
      QString line = "";
      QStringList dataItems;
      QLOG_DEBUG() << "FileParser::getFileData> reading..." << file.fileName();
      QLOG_INFO() << "FileParser::getFileData> separator [" << sep << "]";
      line = in.readLine();
      while (!in.atEnd()) {
        QLOG_DEBUG() << "FileParser::getFileData> reading..." << line ;
        dataItems = line.split(sep);
        arrayHeight++;
        arrayWidth = dataItems.count();
        line = in.readLine();
      }
      QLOG_INFO() << "FileParser::getFileData> arrayHeight = " << arrayHeight
		  << " arrayWidth = " << arrayWidth;
      if ( array ) { free(array); array = NULL;}
      array = (double*) malloc(sizeof(double) * arrayHeight * arrayWidth);
      // Parse the file to array
      in.seek(0);
      line = "";
      int cnt = 0;
      line = in.readLine();
      while (!in.atEnd()) {
        QLOG_DEBUG() << "FileParser::getFileData> grabbing..." << line ;
         dataItems = line.split(sep);
        foreach (const QString &data, dataItems)  {
          QLOG_DEBUG() << "FileParser::getFileData> grabbing [" << data << "]";
          array[cnt++] = data.toDouble();
        }
        success = true;
        line = in.readLine();
      }
    } 
    else if ( type == "LIST" ) {
      // Extract list of data
      QString nameline = "";
      QString dataline = "";
      QStringList nameItems;
      QStringList dataItems;
      dataList.clear();
      // Grab the third line (Phasics)
      nameline = in.readLine();
      nameline = in.readLine();
      nameline = in.readLine();
      // Grab the successive line (Phasics)
      dataline = in.readLine();
      nameItems = nameline.split(sep);
      dataItems = dataline.split(sep);
      int namepos = 0;  
      foreach (const QString &name, nameItems) {
        QLOG_DEBUG() << "FileParser::getFileData> parsed parameter : " <<  name;
        foreach (const QString &nameRef, nameList) {
          if ( nameRef == name ) {
            QString data = dataItems.at(namepos); 
            data.replace(",",".");
            QLOG_INFO() << "FileParser::getFileData> match with parameter : " <<  nameRef << " = " << data;
            dataList.push_back(data.toDouble());
            // Go back to previous line
            if ( dataList.size() == nameList.size() )
               success = true; 
          } 
        }
        namepos++;
      }
    }
    file.close();
  }
  // Check directory content deletion after
   if ( del == "AFTER" ) {
    foreach (const QString &filename, fileList)
      directory.remove( filename );
  }
  return success;
}              

double*
FileParser::getArray()
{
  return array;
}

int
FileParser::getArrayHeight()
{
  return arrayHeight;
}

int
FileParser::getArrayWidth()
{
  return arrayWidth;
}


QVector<double>
FileParser::getDataList()
{
  return dataList;
}

