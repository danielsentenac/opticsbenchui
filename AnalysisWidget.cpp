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

#include "AnalysisWidget.h"
#include "Utils.h"

namespace {
const char kSelectionStylesheet[] =
    "QTreeView::item:selected{background-color: palette(highlight); "
    "color: palette(highlightedText);};";
}  // namespace

AnalysisWidget::AnalysisWidget(QString appDirPath)
    : appDirPath(appDirPath),
      path(),
      analysisrow(0),
      analysistitle(new QLabel("Analysis sequence")),
      statusLabel(new QLabel("Idle")),
      outputView(new QTextEdit()),
      analysistable(nullptr),
      analysisview(new QTableView()),
      reloadButton(new QPushButton("Reload", this)),
      updateButton(new QPushButton("Update", this)),
      removeButton(new QPushButton("Delete", this)),
      runButton(new QPushButton("Run", this)),
      stopButton(new QPushButton("Stop", this)),
      gridlayout(new QGridLayout()),
      analysis(new AnalysisThread()) {
  QLOG_DEBUG() << "AnalysisWidget::AnalysisWidget";

  dbConnexion();

  setLayout(gridlayout);

  gridlayout->addWidget(analysistitle, 0, 0, 1, 10);
  analysistable = new QSqlTableModel(this, QSqlDatabase::database(path));
  analysisview->setStyleSheet(kSelectionStylesheet);
  analysisview->setModel(analysistable);
  analysisview->verticalHeader()->hide();
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

  gridlayout->addWidget(statusLabel, 5, 2, 1, 6);
  outputView->setReadOnly(true);
  outputView->setMinimumHeight(120);
  gridlayout->addWidget(outputView, 6, 0, 1, 10);

  setupAnalysisTable();
  InitConfig();

  connect(analysis, SIGNAL(analysisStarted(int)), this,
          SLOT(analysisStarted(int)));
  connect(analysis, SIGNAL(analysisFinished(int, bool, QString)), this,
          SLOT(analysisFinished(int, bool, QString)));
  connect(analysis, SIGNAL(showWarning(QString)), this,
          SLOT(showAnalysisWarning(QString)));
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
  stop();
  statusLabel->setText("Running...");
  outputView->clear();
  analysis->setTasks(buildTaskList());
  analysis->start();
  QLOG_DEBUG() << "AnalysisWidget::run started";
}

void AnalysisWidget::runFromAcquisition() {
  QLOG_INFO() << "AnalysisWidget::runFromAcquisition";
  run();
}

void AnalysisWidget::stop() {
  analysis->stop();
  statusLabel->setText("Idle");
}

void AnalysisWidget::analysisStarted(int record) {
  statusLabel->setText(QString("Running record %1").arg(record));
  outputView->append(QString(">> Running record %1").arg(record));
}

void AnalysisWidget::analysisFinished(int record, bool success,
                                      const QString& output) {
  QLOG_INFO() << "AnalysisWidget::analysisFinished record=" << record
              << " success=" << success;
  if (!output.trimmed().isEmpty()) {
    QLOG_INFO() << "Analysis output: " << output;
    outputView->append(output);
  }
  statusLabel->setText(success ? "Done" : "Failed");
  outputView->append(success ? ">> Done" : ">> Failed");
}

void AnalysisWidget::showAnalysisWarning(QString message) {
  Utils::EmitWarning(this, __FUNCTION__, message);
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
