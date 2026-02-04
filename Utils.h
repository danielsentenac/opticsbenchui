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
#include <QTableView>
#include <QtSql>

#include "QsLog.h"

/// \ingroup utils
/// Utility helpers for logging, database paths, and timestamps.
namespace Utils {
/// Return the current time in microseconds.
double GetTimeMicroseconds();
/// Build a database path from a base directory and filename.
/// \param baseDir Base directory.
/// \param filename Database filename.
QString BuildDbPath(const QString& baseDir, const QString& filename);
/// Return the current timestamp as a formatted string.
QString CurrentTimestampString();
/// Ensure a filename ends with the HDF5 extension.
/// \param filename Input filename.
QString EnsureHdf5Extension(const QString& filename);
/// Build a default HDF5 path using a base name.
/// \param baseName Base filename.
QString DefaultHdf5Path(const QString& baseName);
/// Return the file dialog filter for HDF5 files.
QString Hdf5FileDialogFilter();
/// Open a SQLite database and log context for errors.
/// \param path Database path.
/// \param logContext Context string for logging.
QSqlDatabase ConnectSqliteDb(const QString& path, const char* logContext);
/// Execute a SQL statement and log on error.
/// \param query Query object.
/// \param sql SQL statement.
/// \param logContext Context string for logging.
bool ExecSql(QSqlQuery& query, const QString& sql, const char* logContext);
/// Log an error with context.
/// \param logContext Context string for logging.
/// \param message Error message.
void ReportError(const char* logContext, const QString& message);
/// Log a warning with context.
/// \param logContext Context string for logging.
/// \param message Warning message.
void ReportWarning(const char* logContext, const QString& message);
/// Apply default sizing and wrapping behavior to SQL table views.
/// \param view Table view to configure.
void ConfigureSqlTableView(QTableView* view);
/// Update row sizing based on current model content.
/// \param view Table view to update.
void UpdateSqlTableViewRowSizing(QTableView* view);
/// Update column sizing based on content and viewport width.
/// \param view Table view to update.
void UpdateSqlTableViewColumnSizing(QTableView* view);

/// Report a warning and emit `showWarning` when available.
/// \param sender Object that may expose `showWarning`.
/// \param logContext Context string for logging.
/// \param message Warning message.
template <typename T>
void EmitWarning(T* sender, const char* logContext, const QString& message) {
  ReportWarning(logContext, message);
  if (sender) {
    sender->showWarning(message);
  }
}
}  // namespace Utils

#endif
