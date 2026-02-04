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
 * @file AnalysisWidget.h
 * @brief Analysis widget.
 */

#ifndef ANALYSISWIDGET_H
#define ANALYSISWIDGET_H

#include <QtSql>
#include <QtWidgets>
#include "QsLog.h"
#include "AnalysisThread.h"

/// \ingroup ui
/// UI widget for analysis tools.
class AnalysisWidget : public QWidget
{
  Q_OBJECT
    
public:
  /// Constructor.
  explicit AnalysisWidget(QString appDirPath = QString());
  /// Destructor.
  ~AnalysisWidget() override;

public slots:
  /// Refresh displayed values.
  void update();
  /// Reload the analysis table.
  void reload();
  /// Remove the selected analysis row.
  void remove();
  /// Start analysis.
  void run();
  /// Stop analysis.
  void stop();
  /// Set the database path used by the widget.
  /// \param path Filesystem path to DBs.
  void setDbPath(QString path);
  /// Run analysis triggered by acquisition.
  void runFromAcquisition();
  /// Update status label when a task starts.
  /// \param record Task record index.
  void analysisStarted(int record);
  /// Update status label when a task finishes.
  /// \param record Task record index.
  /// \param success True if the command succeeded.
  /// \param output Process output.
  void analysisFinished(int record, bool success, const QString& output);
  /// Append streaming output from the analysis process.
  /// \param record Task record index.
  /// \param output Output chunk.
  void analysisOutput(int record, const QString& output);
  /// Show analysis warning to the user.
  /// \param message Warning message.
  void showAnalysisWarning(QString message);

signals:
  /// Emit warnings to the UI.
  /// \param message Warning message.
  void showWarning(QString message);

private:
  void InitConfig();
  void dbConnexion();
  void setupAnalysisTable();
  QVector<AnalysisTask> buildTaskList() const;

  QString appDirPath;
  QString path;
  int analysisrow;

  QLabel* analysistitle;
  QLabel* statusLabel;
  QTextEdit* outputView;
  QSqlTableModel* analysistable;
  QTableView* analysisview;
  QPushButton* reloadButton;
  QPushButton* updateButton;
  QPushButton* removeButton;
  QPushButton* runButton;
  QPushButton* stopButton;
  QGridLayout* gridlayout;

  AnalysisThread* analysis;
};
#endif
