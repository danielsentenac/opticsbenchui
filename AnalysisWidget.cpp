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
 * @file AnalysisWidget.cpp
 * @brief Analysis widget.
 */

#include "AnalysisWidget.h"
#include "Utils.h"

namespace {
const char kSelectionStylesheet[] =
    "QTreeView::item:selected{background-color: palette(highlight); "
    "color: palette(highlightedText);};";
constexpr int kRecordColumn = 0;

QString FormatElapsed(qint64 elapsedMs) {
  const qint64 totalSeconds = elapsedMs / 1000;
  const qint64 hours = totalSeconds / 3600;
  const qint64 minutes = (totalSeconds % 3600) / 60;
  const qint64 seconds = totalSeconds % 60;
  return QString("Elapsed: %1:%2:%3")
      .arg(hours, 2, 10, QLatin1Char('0'))
      .arg(minutes, 2, 10, QLatin1Char('0'))
      .arg(seconds, 2, 10, QLatin1Char('0'));
}

class RecordHighlightDelegate : public QStyledItemDelegate {
 public:
  explicit RecordHighlightDelegate(const AnalysisWidget* widget,
                                   QObject* parent = nullptr)
      : QStyledItemDelegate(parent),
        widget(widget) {}

  void paint(QPainter* painter,
             const QStyleOptionViewItem& option,
             const QModelIndex& index) const override {
    QStyleOptionViewItem opt(option);
    if (index.column() == kRecordColumn && widget != nullptr) {
      const int current = widget->currentRecordForHighlight();
      if (current >= 0 && index.data().toInt() == current) {
        opt.backgroundBrush = QBrush(QColor(72, 96, 140));
        opt.palette.setColor(QPalette::Text, QColor(235, 235, 235));
      }
    }
    QStyledItemDelegate::paint(painter, opt, index);
  }

 private:
  const AnalysisWidget* widget;
};
}  // namespace

AnalysisWidget::AnalysisWidget(QString appDirPath)
    : appDirPath(appDirPath),
      path(),
      analysisrow(0),
      analysistitle(new QLabel("Analysis sequence")),
      statusLabel(new QLabel("Idle")),
      pidLabel(new QLabel("PID: -")),
      elapsedLabel(new QLabel("Elapsed: 00:00:00")),
      outputView(new QTextEdit()),
      analysistable(nullptr),
      analysisview(new QTableView()),
      reloadButton(new QPushButton("Reload", this)),
      updateButton(new QPushButton("Update", this)),
      removeButton(new QPushButton("Delete", this)),
      runButton(new QPushButton("Run", this)),
      stopButton(new QPushButton("Stop", this)),
      gridlayout(new QGridLayout()),
      analysis(new AnalysisThread()),
      elapsedTimerTick(new QTimer(this)),
      currentRecord(-1),
      lastActivePid(0),
      expectedTasks(0),
      finishedTasks(0),
      analysisWasStopped(false) {
  QLOG_DEBUG() << "AnalysisWidget::AnalysisWidget";

  dbConnexion();

  setLayout(gridlayout);

  gridlayout->addWidget(analysistitle, 0, 0, 1, 10);
  analysistable = new QSqlTableModel(this, QSqlDatabase::database(path));
  analysisview->setStyleSheet(kSelectionStylesheet);
  analysisview->setModel(analysistable);
  analysisview->verticalHeader()->hide();
  analysisview->setItemDelegateForColumn(
      kRecordColumn, new RecordHighlightDelegate(this, analysisview));
  Utils::ConfigureSqlTableView(analysisview);
  gridlayout->addWidget(analysisview, 1, 0, 1, 10);

  reloadButton->setFixedSize(100, 30);
  connect(reloadButton, SIGNAL(clicked()), this, SLOT(reload()));
  gridlayout->addWidget(reloadButton, 2, 0, 1, 1);

  updateButton->setFixedSize(100, 30);
  connect(updateButton, SIGNAL(clicked()), this, SLOT(update()));
  gridlayout->addWidget(updateButton, 2, 1, 1, 1);

  removeButton->setFixedSize(100, 30);
  connect(removeButton, SIGNAL(clicked()), this, SLOT(remove()));
  gridlayout->addWidget(removeButton, 2, 2, 1, 1);

  runButton->setFixedSize(100, 30);
  connect(runButton, SIGNAL(clicked()), this, SLOT(run()));
  gridlayout->addWidget(runButton, 5, 0, 1, 1);

  stopButton->setFixedSize(100, 30);
  connect(stopButton, SIGNAL(clicked()), this, SLOT(stop()));
  gridlayout->addWidget(stopButton, 5, 1, 1, 1);

  gridlayout->addWidget(statusLabel, 5, 2, 1, 3);
  gridlayout->addWidget(pidLabel, 5, 5, 1, 1);
  gridlayout->addWidget(elapsedLabel, 5, 6, 1, 4);
  outputView->setReadOnly(true);
  outputView->setMinimumHeight(120);
  gridlayout->addWidget(outputView, 6, 0, 1, 10);

  elapsedTimerTick->setInterval(1000);
  connect(elapsedTimerTick, SIGNAL(timeout()), this, SLOT(updateElapsed()));

  setupAnalysisTable();
  InitConfig();

  connect(analysis, SIGNAL(analysisStarted(int)), this,
          SLOT(analysisStarted(int)));
  connect(analysis, SIGNAL(analysisFinished(int, bool, QString)), this,
          SLOT(analysisFinished(int, bool, QString)));
  connect(analysis, SIGNAL(analysisOutput(int, QString)), this,
          SLOT(analysisOutput(int, QString)));
  connect(analysis, SIGNAL(pidChanged(qint64)), this,
          SLOT(updatePid(qint64)));
  connect(analysis, SIGNAL(showWarning(QString)), this,
          SLOT(showAnalysisWarning(QString)));
  connect(analysis, SIGNAL(finished()), this,
          SLOT(analysisThreadFinished()));
}

AnalysisWidget::~AnalysisWidget() {
  analysis->stop();
  delete analysis;
  QSqlDatabase db = QSqlDatabase::database(path);
  db.close();
  delete analysistable;
  QSqlDatabase::removeDatabase(path);
  QLOG_DEBUG() << "Deleting AnalysisWidget";
}

void AnalysisWidget::setDbPath(QString path) {
  {
    QSqlDatabase db = QSqlDatabase::database(this->path);
    db.close();
    delete analysistable;
  }
  QSqlDatabase::removeDatabase(this->path);

  appDirPath = path;
  dbConnexion();

  analysistable = new QSqlTableModel(this, QSqlDatabase::database(this->path));
  analysisview->setModel(analysistable);
  analysisview->verticalHeader()->hide();
  Utils::ConfigureSqlTableView(analysisview);
  setupAnalysisTable();
  InitConfig();
}

void AnalysisWidget::InitConfig() {
  analysistable->setSort(0, Qt::AscendingOrder);
  analysistable->select();
  analysisrow = analysistable->rowCount();
  analysistable->insertRow(analysisrow);
  analysisview->resizeColumnsToContents();
  analysisview->resizeRowsToContents();
}

void AnalysisWidget::setupAnalysisTable() {
  analysistable->setTable("analysis_sequence");
  analysistable->setEditStrategy(QSqlTableModel::OnManualSubmit);
  analysistable->setHeaderData(0, Qt::Horizontal, tr("record"));
  analysistable->setHeaderData(1, Qt::Horizontal, tr("code path"));
  analysistable->setHeaderData(2, Qt::Horizontal, tr("arguments"));
}

void AnalysisWidget::update() {
  analysistable->submitAll();
  InitConfig();
}

void AnalysisWidget::reload() {
  InitConfig();
}

void AnalysisWidget::remove() {
  QModelIndexList selected;
  QItemSelectionModel *selmodel;
  selmodel = analysisview->selectionModel();
  selected = selmodel->selectedIndexes();
  for (int i = selected.size() - 1; i >= 0; i--) {
    analysistable->removeRow(selected.at(i).row());
  }
  analysistable->submitAll();
  InitConfig();
}

QVector<AnalysisTask> AnalysisWidget::buildTaskList() const {
  QVector<AnalysisTask> tasks;
  QSqlQuery query(QSqlDatabase::database(path));
  query.exec("select record, codepath, arguments from analysis_sequence "
             "order by record");
  while (query.next()) {
    const int record = query.value(0).toInt();
    if (record < 0) {
      continue;
    }
    AnalysisTask task;
    task.record = record;
    task.codePath = query.value(1).toString();
    task.arguments = query.value(2).toString();
    tasks.append(task);
  }
  return tasks;
}

void AnalysisWidget::run() {
  if (analysis->isRunning()) {
    Utils::EmitWarning(this, __FUNCTION__,
                       "Analysis is already running. Stop it before starting a new run.");
    return;
  }
  stop();
  statusLabel->setText("Running...");
  outputView->clear();
  QVector<AnalysisTask> tasks = buildTaskList();
  analysis->setTasks(tasks);
  expectedTasks = tasks.size();
  finishedTasks = 0;
  analysisWasStopped = false;
  elapsedTimer.start();
  elapsedLabel->setText("Elapsed: 00:00:00");
  emit runningChanged(true);
  elapsedTimerTick->start();
  analysis->start();
  QLOG_DEBUG() << "AnalysisWidget::run started";
}

void AnalysisWidget::runFromAcquisition() {
  QLOG_INFO() << "AnalysisWidget::runFromAcquisition";
  if (analysis->isRunning()) {
    Utils::EmitWarning(this, __FUNCTION__,
                       "Analysis is already running. Stop it before starting a new run.");
    return;
  }
  run();
}

void AnalysisWidget::stop() {
  analysis->stop();
  analysisWasStopped = true;
  pidLabel->setText("PID: -");
  emit runningChanged(false);
  statusLabel->setText("Idle");
  elapsedTimerTick->stop();
  currentRecord = -1;
  analysisview->viewport()->update();
  if (elapsedTimer.isValid()) {
    elapsedLabel->setText(FormatElapsed(elapsedTimer.elapsed()));
  }
}

void AnalysisWidget::analysisStarted(int record) {
  statusLabel->setText(QString("Running record %1").arg(record));
  outputView->append(QString(">> Running record %1").arg(record));
  currentRecord = record;
  analysisview->viewport()->update();
}

void AnalysisWidget::analysisFinished(int record, bool success,
                                      const QString& output) {
  QLOG_INFO() << "AnalysisWidget::analysisFinished record=" << record
              << " success=" << success;
  if (!output.trimmed().isEmpty()) {
    QLOG_INFO() << "Analysis output: " << output;
    outputView->append(output);
  }
  if (analysisWasStopped && !success) {
    statusLabel->setText("Stopped");
    outputView->append(QString(">> PID %1 killed")
                           .arg(lastActivePid > 0 ? lastActivePid : 0));
  } else {
    statusLabel->setText(success ? "Done" : "Failed");
    outputView->append(success ? ">> Done" : ">> Failed");
  }
  if (expectedTasks > 0) {
    finishedTasks++;
    if (finishedTasks >= expectedTasks && elapsedTimer.isValid()) {
      elapsedLabel->setText(FormatElapsed(elapsedTimer.elapsed()));
    }
  }
}

void AnalysisWidget::analysisOutput(int record, const QString& output) {
  Q_UNUSED(record);
  if (output.isEmpty()) {
    return;
  }
  outputView->moveCursor(QTextCursor::End);
  outputView->insertPlainText(output);
  outputView->ensureCursorVisible();
}

void AnalysisWidget::showAnalysisWarning(QString message) {
  Utils::EmitWarning(this, __FUNCTION__, message);
}

void AnalysisWidget::analysisThreadFinished() {
  elapsedTimerTick->stop();
  if (elapsedTimer.isValid()) {
    elapsedLabel->setText(FormatElapsed(elapsedTimer.elapsed()));
  }
  statusLabel->setText(analysisWasStopped ? "Stopped" : "Finished");
  pidLabel->setText("PID: -");
  currentRecord = -1;
  analysisview->viewport()->update();
  emit runningChanged(false);
}

void AnalysisWidget::updatePid(qint64 pid) {
  if (pid <= 0) {
    pidLabel->setText("PID: -");
  } else {
    pidLabel->setText(QString("PID: %1").arg(pid));
    lastActivePid = pid;
  }
}

void AnalysisWidget::updateElapsed() {
  if (!elapsedTimer.isValid()) {
    return;
  }
  elapsedLabel->setText(FormatElapsed(elapsedTimer.elapsed()));
}

void AnalysisWidget::dbConnexion() {
  QLOG_DEBUG() << "AnalysisWidget::dbConnexion";
  path = Utils::BuildDbPath(appDirPath, "analysis.db3");
  QSqlDatabase db = Utils::ConnectSqliteDb(path,
                                           "AnalysisWidget::dbConnexion>");
  if (!db.isOpen()) {
    Utils::EmitWarning(this, __FUNCTION__,
                       db.lastError().text());
  }
  QSqlQuery query(QSqlDatabase::database(path));
  Utils::ExecSql(
      query,
      "create table if not exists analysis_sequence "
      "(record int primary key, "
      "codepath varchar(255), "
      "arguments varchar(255))",
      "AnalysisWidget::dbConnexion>");
}
