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

#include "Utils.h"

#include <sys/time.h>

#include <QAbstractItemModel>
#include <QHeaderView>

namespace Utils {
double GetTimeMicroseconds() {
  struct timeval tp;
  gettimeofday(&tp, nullptr);
  return tp.tv_sec * 1e6 + tp.tv_usec;
}

QString BuildDbPath(const QString& baseDir, const QString& filename) {
  QString path = baseDir;
  path.append(QDir::separator()).append(filename);
  return QDir::toNativeSeparators(path);
}

QString CurrentTimestampString() {
  return QDateTime::currentDateTime().toString("MMMdd,yy-hh:mm:ss");
}

QString EnsureHdf5Extension(const QString& filename) {
  if (filename.endsWith(".h5", Qt::CaseInsensitive)) {
    return filename;
  }
  return filename + ".h5";
}

QString DefaultHdf5Path(const QString& baseName) {
  QString name = baseName;
  if (name.isEmpty()) {
    name = "untitled";
  }
  name = EnsureHdf5Extension(name);
  return QDir::currentPath() + QDir::separator() + name;
}

QString Hdf5FileDialogFilter() {
  return "HDF5 (*.h5)";
}

QSqlDatabase ConnectSqliteDb(const QString& path, const char* logContext) {
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", path);
  if (logContext) {
    QLOG_INFO() << logContext << " Db path : " << path;
  } else {
    QLOG_INFO() << "Db path : " << path;
  }
  db.setDatabaseName(path);
  if (!db.open()) {
    QLOG_WARN() << db.lastError().text();
  }
  return db;
}

bool ExecSql(QSqlQuery& query, const QString& sql, const char* logContext) {
  if (!query.exec(sql)) {
    ReportWarning(logContext, query.lastError().text());
    return false;
  }
  return true;
}

void ReportError(const char* logContext, const QString& message) {
  if (logContext) {
    QLOG_ERROR() << logContext << message;
  } else {
    QLOG_ERROR() << message;
  }
}

void ReportWarning(const char* logContext, const QString& message) {
  if (logContext) {
    QLOG_WARN() << logContext << message;
  } else {
    QLOG_WARN() << message;
  }
}

namespace {
bool ModelHasLongText(QAbstractItemModel* model, int threshold) {
  if (!model) {
    return false;
  }
  const int rows = model->rowCount();
  const int cols = model->columnCount();
  const int maxCells = 500;
  int checked = 0;
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < cols; ++c) {
      if (checked++ >= maxCells) {
        return false;
      }
      const QModelIndex idx = model->index(r, c);
      const QVariant data = model->data(idx, Qt::DisplayRole);
      if (!data.isValid()) {
        continue;
      }
      const QString text = data.toString();
      if (text.contains('\n') || text.size() > threshold) {
        return true;
      }
    }
  }
  return false;
}
}  // namespace

void ConfigureSqlTableView(QTableView* view) {
  if (!view) {
    return;
  }
  view->setWordWrap(true);
  view->setTextElideMode(Qt::ElideNone);
  view->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
  view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  if (QHeaderView* header = view->horizontalHeader()) {
    header->setSectionResizeMode(QHeaderView::Stretch);
  }
  UpdateSqlTableViewRowSizing(view);

  QAbstractItemModel* model = view->model();
  if (!model) {
    return;
  }
  auto update = [view]() { UpdateSqlTableViewRowSizing(view); };
  QObject::connect(model, &QAbstractItemModel::dataChanged, view, update);
  QObject::connect(model, &QAbstractItemModel::modelReset, view, update);
  QObject::connect(model, &QAbstractItemModel::layoutChanged, view, update);
  QObject::connect(model, &QAbstractItemModel::rowsInserted, view, update);
  QObject::connect(model, &QAbstractItemModel::rowsRemoved, view, update);
  QObject::connect(model, &QAbstractItemModel::columnsInserted, view, update);
  QObject::connect(model, &QAbstractItemModel::columnsRemoved, view, update);
}

void UpdateSqlTableViewRowSizing(QTableView* view) {
  if (!view) {
    return;
  }
  QAbstractItemModel* model = view->model();
  if (!model) {
    return;
  }
  const bool useWrap = ModelHasLongText(model, 60);
  if (QHeaderView* header = view->verticalHeader()) {
    if (useWrap) {
      header->setSectionResizeMode(QHeaderView::ResizeToContents);
      view->resizeRowsToContents();
    } else {
      header->setSectionResizeMode(QHeaderView::Stretch);
    }
  }
}
}  // namespace Utils
