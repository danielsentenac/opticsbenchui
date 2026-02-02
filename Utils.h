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

#ifndef UTILS_H
#define UTILS_H

#include <QDateTime>
#include <QDir>
#include <QtSql>

#include "QsLog.h"

namespace Utils {
double GetTimeMicroseconds();
QString BuildDbPath(const QString& baseDir, const QString& filename);
QString CurrentTimestampString();
QString EnsureHdf5Extension(const QString& filename);
QString DefaultHdf5Path(const QString& baseName);
QString Hdf5FileDialogFilter();
QSqlDatabase ConnectSqliteDb(const QString& path, const char* logContext);
bool ExecSql(QSqlQuery& query, const QString& sql, const char* logContext);
void ReportError(const char* logContext, const QString& message);
void ReportWarning(const char* logContext, const QString& message);

template <typename T>
void EmitWarning(T* sender, const char* logContext, const QString& message) {
  ReportWarning(logContext, message);
  if (sender) {
    sender->showWarning(message);
  }
}
}  // namespace Utils

#endif
