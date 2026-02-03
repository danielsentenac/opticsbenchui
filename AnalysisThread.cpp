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

#include "AnalysisThread.h"

AnalysisThread::AnalysisThread(QObject* parent)
    : QThread(parent),
      tasks(),
      mutex(new QMutex()),
      process(nullptr),
      suspend(true) {}

AnalysisThread::~AnalysisThread() {
  QLOG_DEBUG() << "Deleting AnalysisThread";
  stop();
  delete mutex;
}

void AnalysisThread::setTasks(const QVector<AnalysisTask>& tasks) {
  this->tasks = tasks;
}

void AnalysisThread::stop() {
  QMutexLocker locker(mutex);
  suspend = true;
  if (process) {
    process->terminate();
    process->waitForFinished(2000);
    if (process->state() != QProcess::NotRunning) {
      process->kill();
      process->waitForFinished(2000);
    }
  }
  wait();
  exit();
}

QStringList AnalysisThread::splitArguments(const QString& arguments) const {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
  return QProcess::splitCommand(arguments);
#else
  return arguments.split(' ', Qt::SkipEmptyParts);
#endif
}

void AnalysisThread::run() {
  {
    QMutexLocker locker(mutex);
    if (!suspend) {
      return;
    }
    suspend = false;
  }

  for (const AnalysisTask& task : tasks) {
    {
      QMutexLocker locker(mutex);
      if (suspend) {
        break;
      }
    }
    if (task.codePath.trimmed().isEmpty()) {
      emit showWarning("Analysis task has empty code path; skipping.");
      continue;
    }

    emit analysisStarted(task.record);
    QLOG_INFO() << "AnalysisThread::run> Starting task " << task.record
                << " cmd=" << task.codePath << " args=" << task.arguments;

    QProcess localProcess;
    process = &localProcess;
    localProcess.setProcessChannelMode(QProcess::MergedChannels);
    localProcess.start(task.codePath, splitArguments(task.arguments));

    if (!localProcess.waitForStarted()) {
      QString output = localProcess.readAllStandardOutput();
      emit analysisFinished(task.record, false, output);
      emit showWarning("Unable to start analysis command: " + task.codePath);
      continue;
    }

    while (!localProcess.waitForFinished(100)) {
      QMutexLocker locker(mutex);
      if (suspend) {
        break;
      }
    }

    const QString output = localProcess.readAllStandardOutput();
    const bool success =
        (localProcess.exitStatus() == QProcess::NormalExit &&
         localProcess.exitCode() == 0);
    emit analysisFinished(task.record, success, output);
  }

  process = nullptr;
  QMutexLocker locker(mutex);
  suspend = true;
  QLOG_DEBUG() << "AnalysisThread::run> End of thread";
}
