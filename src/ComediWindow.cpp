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
 * @file ComediWindow.cpp
 * @brief Comedi window.
 */

#include "ComediWindow.h"
#include "Utils.h"

namespace {
const char kSelectionStylesheet[] =
    "QTreeView::item:selected{background-color: palette(highlight); "
    "color: palette(highlightedText);};";
}  // namespace

ComediWindow::ComediWindow(QMainWindow* parent, Qt::WindowFlags fl,
                           Comedi *comedi)
    : QMainWindow(parent, fl),
      comediWidget(nullptr),
      daccomediWidget(nullptr),
      parentWindow(parent),
      dbPath(),
      comedititle(new QLabel("Comedi settings")),
      comeditable(nullptr),
      comediview(new QTableView()),
      comedirow(0),
      vboxlayout(new QVBoxLayout()),
      updateButton(new QPushButton("Update", this)),
      removeButton(new QPushButton("Delete", this)),
      loadButton(new QPushButton("Load", this)),
      comediList(new QVector<QString>()),
      dockWidget(new QDockWidget(tr("Comedi Control"), this)),
      comedi(comedi) {
  if (this->comedi) {
    dbPath = this->comedi->path;
  }
  QLOG_INFO() << "ComediWindow::ComediWindow> get Db path " << dbPath;
  QWidget *centralWidget = new QWidget(this);
  centralWidget->setMinimumSize(20, 20);
  setCentralWidget(centralWidget);

  centralWidget->setLayout(vboxlayout);

  setupDockWidget();

  // 'comedi_settings' table model
  vboxlayout->addWidget(comedititle);
  comeditable = new QSqlTableModel(this, QSqlDatabase::database(dbPath));
  comediview->setStyleSheet(kSelectionStylesheet);
  comediview->setModel(comeditable);
  Utils::ConfigureSqlTableView(comediview);
  vboxlayout->addWidget(comediview);

  updateButton->setFixedHeight(30);
  connect(updateButton, SIGNAL(clicked()), this, SLOT(update()));
  vboxlayout->addWidget(updateButton);
  removeButton->setFixedHeight(30);
  connect(removeButton, SIGNAL(clicked()), this, SLOT(remove()));
  vboxlayout->addWidget(removeButton);
  loadButton->setFixedHeight(30);
  connect(loadButton, SIGNAL(clicked()), this, SLOT(load()));
  vboxlayout->addWidget(loadButton);
  setWindowTitle("Comedi");

  setupTableModel();
  InitConfig();
}

ComediWindow::~ComediWindow() {
  QLOG_DEBUG() << "Deleting ComediWindow";
  delete comeditable;
  delete comediWidget;
  delete daccomediWidget;
}

void ComediWindow::setDbPath(QString path) {
  delete comeditable;

  if (comedi) {
    comedi->setDbPath(path);
    dbPath = comedi->path;
  }
  comeditable = new QSqlTableModel(this, QSqlDatabase::database(dbPath));
  comediview->setModel(comeditable);
  Utils::ConfigureSqlTableView(comediview);
  setupTableModel();
  InitConfig();
}

void ComediWindow::closeEvent(QCloseEvent* event) {
  event->accept();
  QLOG_DEBUG() << "Closing ComediWindow";
  if (comedi && comedi->comeditype == "COMEDICOUNTER") {
    for (int i = 0; i < comediWidget->setButtonList->size(); i++) {
      QPushButton *button = comediWidget->setButtonList->at(i);
      button->setChecked(false);
    }
  }
  hide();
}

void ComediWindow::InitConfig() {
   // Set where clause
  // QLineEdit shows person lastname that id is given parameter
  //comeditable->setFilter(QString("name=*").arg(name));
  comeditable->select();
  comedirow = comeditable->rowCount();
  comeditable->insertRow(comedirow);
  Utils::UpdateSqlTableViewColumnSizing(comediview);
  Utils::UpdateSqlTableViewRowSizing(comediview);

}

void ComediWindow::update() {
  comeditable->submitAll();
  InitConfig();
}
void ComediWindow::load() {
  InitRun();
}

void ComediWindow::remove() {
  QModelIndexList selected;
  QItemSelectionModel *selmodel;
  selmodel = comediview->selectionModel();
  selected = selmodel->selectedIndexes();
  for (int i = selected.size() - 1; i >= 0; i--) {
    comeditable->removeRow(selected.at(i).row());
  }
  comeditable->submitAll();
  InitConfig();
}

void ComediWindow::InitRun() {
  comediList->clear();
  QSqlDatabase db = QSqlDatabase::database(dbPath);
  QSqlQuery query(db);
  query.exec("select name from comedi_settings");
  while (query.next()) {
    QString name = query.value(0).toString();
    comediList->append(name);
  }
  if (comedi->comeditype == "COMEDICOUNTER") {
    comediWidget->setComediList(comediList);
  }
  else if (comedi->comeditype == "COMEDIDAC") {
    daccomediWidget->setComediList(comediList);
  }
}

void ComediWindow::setupTableModel() {
  comeditable->setTable("comedi_settings");
  comeditable->setEditStrategy(QSqlTableModel::OnManualSubmit);
  comeditable->setHeaderData(0, Qt::Horizontal, tr("name"));
  comeditable->setHeaderData(1, Qt::Horizontal, tr("settings"));
  comeditable->setHeaderData(2, Qt::Horizontal, tr("description"));
  comeditable->setHeaderData(3, Qt::Horizontal, tr("comedivalues"));
  comediview->setColumnHidden(3, true);
}

void ComediWindow::setupDockWidget() {
  if (!comedi) {
    return;
  }
  if (comedi->comeditype == "COMEDICOUNTER") {
    comediWidget = new ComediCounterControlWidget(comediList);
    comediWidget->setComedi(comedi);
    dockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
    QScrollArea *scrollableWidget = new QScrollArea();
    scrollableWidget->setWidget(comediWidget);
    scrollableWidget->setWidgetResizable(true);
    dockWidget->setWidget(scrollableWidget);
    addDockWidget(Qt::BottomDockWidgetArea, dockWidget);
  }
  else if (comedi->comeditype == "COMEDIDAC") {
    daccomediWidget = new ComediDacControlWidget(comediList);
    daccomediWidget->setComedi(comedi);
    dockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
    QScrollArea *scrollableWidget = new QScrollArea();
    scrollableWidget->setWidget(daccomediWidget);
    scrollableWidget->setWidgetResizable(true);
    dockWidget->setWidget(scrollableWidget);
    addDockWidget(Qt::BottomDockWidgetArea, dockWidget);
  }
}
