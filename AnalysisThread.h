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

#ifndef ANALYSISTHREAD_H
#define ANALYSISTHREAD_H

#include <QtCore>
#include "QsLog.h"

/// \ingroup analysis
/// Analysis task descriptor from the analysis table.
struct AnalysisTask {
  /// Task order / record index.
  int record = 0;
  /// Full path to executable or script.
  QString codePath;
  /// Raw argument string (split by QProcess).
  QString arguments;
};

/// Executes external analysis commands in a worker thread.
///
/// Each task is run using QProcess with the specified code path and
/// argument string.
class AnalysisThread : public QThread
{
  Q_OBJECT

public:
  /// Construct the analysis thread.
  explicit AnalysisThread(QObject* parent = nullptr);
  /// Destructor.
  ~AnalysisThread() override;

  /// Set the task list to run.
  /// \param tasks Analysis tasks to execute.
  void setTasks(const QVector<AnalysisTask>& tasks);
  /// Request the thread to stop.
  void stop();

signals:
  /// Emitted when a task starts.
  /// \param record Task record index.
  void analysisStarted(int record);
  /// Emitted when a task finishes.
  /// \param record Task record index.
  /// \param success True if the command exited normally with code 0.
  /// \param output Combined stdout/stderr output.
  void analysisFinished(int record, bool success, const QString& output);
  /// Emit warnings to the UI.
  /// \param message Warning message.
  void showWarning(QString message);

protected:
  /// Thread entry point.
  void run() override;

private:
  QStringList splitArguments(const QString& arguments) const;

  QVector<AnalysisTask> tasks;
  QMutex* mutex;
  QProcess* process;
  bool suspend;
};

#endif // ANALYSISTHREAD_H
