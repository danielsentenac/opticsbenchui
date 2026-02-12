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

/**
 * @file Utils.cpp
 * @brief Utility helpers.
 */

#include "Utils.h"

#include <chrono>

#include <QAbstractItemModel>
#include <QFontMetrics>
#include <QHeaderView>

namespace Utils {
double GetTimeMicroseconds() {
  const auto now = std::chrono::system_clock::now().time_since_epoch();
  return static_cast<double>(
      std::chrono::duration_cast<std::chrono::microseconds>(now).count());
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
QVector<int> VisibleColumnIndices(QTableView* view, int columns) {
  QVector<int> visible;
  if (!view || columns <= 0) {
    return visible;
  }
  visible.reserve(columns);
  for (int c = 0; c < columns; ++c) {
    if (!view->isColumnHidden(c)) {
      visible.push_back(c);
    }
  }
  return visible;
}

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

bool ModelHasWrappedContent(QTableView* view) {
  if (!view) {
    return false;
  }
  QAbstractItemModel* model = view->model();
  if (!model) {
    return false;
  }
  const int rows = model->rowCount();
  const int cols = model->columnCount();
  const int maxCells = 500;
  int checked = 0;
  const QFontMetrics metrics(view->font());
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
      if (text.contains('\n')) {
        return true;
      }
      if (metrics.horizontalAdvance(text) > view->columnWidth(c)) {
        return true;
      }
    }
  }
  return false;
}
}  // namespace

namespace {
class SqlTableViewResizer : public QObject {
 public:
  explicit SqlTableViewResizer(QTableView* view)
      : QObject(view),
        view_(view),
        in_adjust_(false),
        user_resized_(false) {}

 protected:
  bool eventFilter(QObject* obj, QEvent* event) override {
    if ((obj == view_ || obj == view_->viewport()) &&
        (event->type() == QEvent::Resize || event->type() == QEvent::Show)) {
      if (view_ && !in_adjust_) {
        if (!user_resized_) {
          UpdateSqlTableViewColumnSizing(view_);
        } else {
          AdjustColumnsToViewport(view_, -1);
        }
        UpdateSqlTableViewRowSizing(view_);
      }
    }
    return QObject::eventFilter(obj, event);
  }

 public:
  void OnHeaderSectionResized(int logicalIndex) {
    if (in_adjust_) {
      return;
    }
    user_resized_ = true;
    in_adjust_ = true;
    AdjustColumnsToViewport(view_, logicalIndex);
    UpdateSqlTableViewRowSizing(view_);
    in_adjust_ = false;
  }

  bool ShouldAutoSize() const {
    return !user_resized_;
  }

 private:
  void AdjustColumnsToViewport(QTableView* view, int resizedColumn) {
    if (!view) {
      return;
    }
    QHeaderView* header = view->horizontalHeader();
    if (!header) {
      return;
    }
    const int columns = header->count();
    if (columns <= 0) {
      return;
    }
    const QVector<int> visible = VisibleColumnIndices(view, columns);
    if (visible.isEmpty()) {
      return;
    }
    int total = 0;
    for (int c : visible) {
      total += header->sectionSize(c);
    }
    const int viewportWidth = view->viewport()->width();
    if (viewportWidth > total) {
      const int extra = viewportWidth - total;
      QVector<int> indices;
      indices.reserve(visible.size());
      for (int c : visible) {
        if (c != resizedColumn) {
          indices.push_back(c);
        }
      }
      if (indices.isEmpty()) {
        header->resizeSection(resizedColumn,
                              header->sectionSize(resizedColumn) + extra);
        return;
      }
      int weightSum = 0;
      for (int idx : indices) {
        weightSum += qMax(header->sectionSize(idx), 1);
      }
      int distributed = 0;
      for (int idx : indices) {
        const int add = (extra * qMax(header->sectionSize(idx), 1)) /
                        qMax(weightSum, 1);
        header->resizeSection(idx, header->sectionSize(idx) + add);
        distributed += add;
      }
      if (distributed < extra) {
        int target = indices.last();
        header->resizeSection(target,
                              header->sectionSize(target) + (extra - distributed));
      }
    }
    if (viewportWidth < total) {
      const int deficit = total - viewportWidth;
      QVector<int> indices;
      indices.reserve(visible.size());
      for (int c : visible) {
        if (c != resizedColumn) {
          indices.push_back(c);
        }
      }
      if (indices.isEmpty()) {
        return;
      }
      int weightSum = 0;
      for (int idx : indices) {
        weightSum += qMax(header->sectionSize(idx), 1);
      }
      int distributed = 0;
      for (int idx : indices) {
        int sub = (deficit * qMax(header->sectionSize(idx), 1)) /
                  qMax(weightSum, 1);
        int newSize = qMax(header->minimumSectionSize(),
                           header->sectionSize(idx) - sub);
        distributed += (header->sectionSize(idx) - newSize);
        header->resizeSection(idx, newSize);
      }
      if (distributed < deficit) {
        int target = indices.last();
        int newSize = qMax(header->minimumSectionSize(),
                           header->sectionSize(target) - (deficit - distributed));
        header->resizeSection(target, newSize);
      }
    }
  }

 private:
  QTableView* view_;
  bool in_adjust_;
  bool user_resized_;
};
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
  view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  SqlTableViewResizer* resizer = nullptr;
  if (QHeaderView* header = view->horizontalHeader()) {
    header->setSectionResizeMode(QHeaderView::Interactive);
    const bool stretchLast =
        view->property("stretchLastColumn").toBool();
    header->setStretchLastSection(stretchLast);
    resizer = new SqlTableViewResizer(view);
    view->installEventFilter(resizer);
    if (view->viewport()) {
      view->viewport()->installEventFilter(resizer);
    }
    QObject::connect(header, &QHeaderView::sectionResized, view,
                     [resizer](int logicalIndex, int, int) {
                       resizer->OnHeaderSectionResized(logicalIndex);
                     });
  }
  UpdateSqlTableViewColumnSizing(view);
  UpdateSqlTableViewRowSizing(view);

  QAbstractItemModel* model = view->model();
  if (!model) {
    return;
  }
  // event filter installed above to handle resize and column adjustments
  auto update = [view, resizer]() {
    const bool disableAutoResizeOnDataChange =
        view->property("disableAutoResizeOnDataChange").toBool();
    if (!disableAutoResizeOnDataChange) {
      if (!resizer || resizer->ShouldAutoSize()) {
        UpdateSqlTableViewColumnSizing(view);
      }
    }
    UpdateSqlTableViewRowSizing(view);
  };
  QObject::connect(model, &QAbstractItemModel::dataChanged, view, update);
  QObject::connect(model, &QAbstractItemModel::modelReset, view, update);
  QObject::connect(model, &QAbstractItemModel::layoutChanged, view, update);
  QObject::connect(model, &QAbstractItemModel::rowsInserted, view, update);
  QObject::connect(model, &QAbstractItemModel::rowsRemoved, view, update);
  QObject::connect(model, &QAbstractItemModel::columnsInserted, view, update);
  QObject::connect(model, &QAbstractItemModel::columnsRemoved, view, update);
}

void UpdateSqlTableViewColumnSizing(QTableView* view) {
  if (!view) {
    return;
  }
  QAbstractItemModel* model = view->model();
  if (!model) {
    return;
  }
  const int columns = model->columnCount();
  if (columns <= 0) {
    return;
  }
  QHeaderView* header = view->horizontalHeader();
  if (!header) {
    return;
  }
  const QVector<int> visible = VisibleColumnIndices(view, columns);
  if (visible.isEmpty()) {
    return;
  }

  const bool stretchLast = view->property("stretchLastColumn").toBool();
  const int fixedLastWidth = view->property("fixedLastColumnWidth").toInt();
  const bool hasFixedLast = fixedLastWidth > 0;
  const bool disableLastExpand =
      view->property("disableLastColumnExpand").toBool();
  const int configuredMaxWidth = view->property("maxAutoColumnWidth").toInt();
  const int viewportWidth = qMax(view->viewport()->width(), 0);
  const int adaptiveMaxWidth =
      viewportWidth > 0 ? qMax(220, viewportWidth * 45 / 100) : 520;
  const int maxAutoColumnWidth =
      configuredMaxWidth > 0 ? configuredMaxWidth : adaptiveMaxWidth;
  header->setStretchLastSection(stretchLast);
  const int lastVisibleColumn = visible.last();
  if (hasFixedLast) {
    header->setSectionResizeMode(lastVisibleColumn, QHeaderView::Fixed);
  }
  view->resizeColumnsToContents();
  if (hasFixedLast) {
    view->setColumnWidth(lastVisibleColumn, fixedLastWidth);
  }
  // Ensure headers remain readable.
  QVector<int> headerMinWidths;
  headerMinWidths.reserve(columns);
  for (int c = 0; c < columns; ++c) {
    const QString headerText = model->headerData(c, Qt::Horizontal).toString();
    const int headerWidth =
        QFontMetrics(header->font()).horizontalAdvance(headerText) + 24;
    const int minWidth = qMax(headerWidth, header->minimumSectionSize());
    headerMinWidths.push_back(minWidth);
    header->resizeSection(c, qMax(header->sectionSize(c), minWidth));
  }
  QVector<int> widths(columns, 0);
  int total = 0;
  for (int c : visible) {
    int width = qMax(header->sectionSize(c), headerMinWidths[c]);
    if (!(hasFixedLast && c == lastVisibleColumn)) {
      width = qMin(width, maxAutoColumnWidth);
    }
    widths[c] = width;
    total += width;
  }
  if (stretchLast) {
    int used = 0;
    for (int c : visible) {
      if (c == lastVisibleColumn) {
        continue;
      }
      used += widths[c];
      view->setColumnWidth(c, widths[c]);
    }
    const int lastWidth =
        hasFixedLast
            ? qMax(fixedLastWidth, headerMinWidths[lastVisibleColumn])
            : qMax(headerMinWidths[lastVisibleColumn], viewportWidth - used);
    view->setColumnWidth(lastVisibleColumn, lastWidth);
    return;
  }

  if (viewportWidth > 0 && total > viewportWidth) {
    int shrinkable = 0;
    for (int c : visible) {
      if (hasFixedLast && c == lastVisibleColumn) {
        continue;
      }
      shrinkable += qMax(0, widths[c] - headerMinWidths[c]);
    }
    int deficit = total - viewportWidth;
    if (shrinkable > 0) {
      int distributed = 0;
      for (int c : visible) {
        if (hasFixedLast && c == lastVisibleColumn) {
          continue;
        }
        const int capacity = qMax(0, widths[c] - headerMinWidths[c]);
        if (capacity <= 0) {
          continue;
        }
        const int delta = qMin(capacity, (deficit * capacity) / shrinkable);
        widths[c] -= delta;
        distributed += delta;
      }
      int remainder = deficit - distributed;
      while (remainder > 0) {
        bool changed = false;
        for (int c : visible) {
          if (hasFixedLast && c == lastVisibleColumn) {
            continue;
          }
          if (widths[c] > headerMinWidths[c]) {
            widths[c]--;
            remainder--;
            changed = true;
            if (remainder == 0) {
              break;
            }
          }
        }
        if (!changed) {
          break;
        }
      }
    }
  } else if (!disableLastExpand && !hasFixedLast && viewportWidth > total) {
    widths[lastVisibleColumn] += (viewportWidth - total);
  }

  for (int c : visible) {
    const int width =
        (hasFixedLast && c == lastVisibleColumn)
            ? qMax(fixedLastWidth, headerMinWidths[c])
            : qMax(widths[c], headerMinWidths[c]);
    view->setColumnWidth(c, width);
  }
}

void UpdateSqlTableViewRowSizing(QTableView* view) {
  if (!view) {
    return;
  }
  QAbstractItemModel* model = view->model();
  if (!model) {
    return;
  }
  const bool useWrap = ModelHasLongText(model, 60) || ModelHasWrappedContent(view);
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
