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

#include "DacWindow.h"

namespace {
const char kSelectionStylesheet[] =
    "QTreeView::item:selected{background-color: palette(highlight); "
    "color: palette(highlightedText);};";
}  // namespace

DacWindow::DacWindow(QMainWindow* parent, Qt::WindowFlags fl, Dac *dac)
    : QMainWindow(parent, fl),
      parentWindow(parent),
      dbPath(),
      dactitle(new QLabel("Dac settings")),
      dactable(nullptr),
      dacview(new QTableView()),
      dacrow(0),
      vboxlayout(new QVBoxLayout()),
      updateButton(new QPushButton("Update", this)),
      removeButton(new QPushButton("Delete", this)),
      loadButton(new QPushButton("Load", this)),
      dacList(new QVector<QString>()),
      dockWidget(new QDockWidget(tr("Dac Control"), this)),
      dac(dac) {
  if (this->dac) {
    dbPath = this->dac->path;
  }
  QLOG_INFO() << "DacWindow::DacWindow> get Db path " << dbPath;
  QWidget *centralWidget = new QWidget(this);

  setCentralWidget(centralWidget);
  centralWidget->setLayout(vboxlayout);

  dacWidget = new DacControlWidget(dacList);
  dacWidget->setDac(this->dac);
  dockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
  QScrollArea *scrollableWidget = new QScrollArea();
  scrollableWidget->setWidget(dacWidget);
  scrollableWidget->setWidgetResizable(true);
  dockWidget->setWidget(scrollableWidget);
  addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

  // 'dac_settings' table model
  vboxlayout->addWidget(dactitle);
  dactable = new QSqlTableModel(this, QSqlDatabase::database(dbPath));
  dacview->setStyleSheet(kSelectionStylesheet);
  dacview->setModel(dactable);
  vboxlayout->addWidget(dacview);

  updateButton->setFixedHeight(30);
  connect(updateButton, SIGNAL(clicked()), this, SLOT(update()));
  vboxlayout->addWidget(updateButton);
  removeButton->setFixedHeight(30);
  connect(removeButton, SIGNAL(clicked()), this, SLOT(remove()));
  vboxlayout->addWidget(removeButton);
  loadButton->setFixedHeight(30);
  connect(loadButton, SIGNAL(clicked()), this, SLOT(load()));
  vboxlayout->addWidget(loadButton);
  setWindowTitle("Dac");

  setupTableModel();
  InitConfig();
}

DacWindow::~DacWindow() {
  QLOG_DEBUG() << "Deleting DacWindow";
  delete dactable;
  delete dac;
}

void DacWindow::setDbPath(QString path) {
  delete dactable;

  if (dac) {
    dac->setDbPath(path);
    dbPath = dac->path;
  }
  dactable = new QSqlTableModel(this, QSqlDatabase::database(dbPath));
  dacview->setModel(dactable);
  setupTableModel();
  InitConfig();
}

void DacWindow::closeEvent(QCloseEvent* event) {
  event->accept();
  QLOG_DEBUG() << "Closing DacWindow";
  hide();
}

void DacWindow::InitConfig() {
   // Set where clause
  // QLineEdit shows person lastname that id is given parameter
  //dactable->setFilter(QString("name=*").arg(name));
  dactable->select();
  dacrow = dactable->rowCount();
  dactable->insertRow(dacrow);
  dacview->resizeColumnsToContents();
  dacview->resizeRowsToContents();

}

void DacWindow::update() {
  dactable->submitAll();
  InitConfig();
}
void DacWindow::load() {
  InitRun();
}

void DacWindow::remove() {
  QModelIndexList selected;
  QItemSelectionModel *selmodel;
  selmodel = dacview->selectionModel();
  selected = selmodel->selectedIndexes();
  for (int i = selected.size() - 1; i >= 0; i--) {
    dactable->removeRow(selected.at(i).row());
  }
  dactable->submitAll();
  InitConfig();
}

void DacWindow::InitRun() {
  dacList->clear();
  QSqlDatabase db = QSqlDatabase::database(dbPath);
  QSqlQuery query(db);
  query.exec("select name from dac_settings");
  while (query.next()) {
    QString name = query.value(0).toString();
    dacList->append(name);
  }
  dacWidget->setDacList(dacList);
}

void DacWindow::setupTableModel() {
  dactable->setTable("dac_settings");
  dactable->setEditStrategy(QSqlTableModel::OnManualSubmit);
  dactable->setHeaderData(0, Qt::Horizontal, tr("name"));
  dactable->setHeaderData(1, Qt::Horizontal, tr("settings"));
  dactable->setHeaderData(2, Qt::Horizontal, tr("description"));
  dactable->setHeaderData(3, Qt::Horizontal, tr("dacvalues"));
  dacview->setColumnHidden(3, true);
}
