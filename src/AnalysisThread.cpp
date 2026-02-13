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
 * @file AnalysisThread.cpp
 * @brief Analysis worker thread.
 */

#include "AnalysisThread.h"
#include "PosixCompat.h"
#include <signal.h>
#include <errno.h>
#include <QtCore/QFileInfo>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QStandardPaths>

namespace {
#ifdef Q_OS_UNIX
bool IsProcessAlive(qint64 pid) {
  if (pid <= 0) {
    return false;
  }
  if (::kill(static_cast<pid_t>(pid), 0) == 0) {
    return true;
  }
  return errno == EPERM;
}

void KillProcessTree(qint64 pid, int sig) {
  if (pid <= 0) {
    return;
  }
  const pid_t target = static_cast<pid_t>(pid);
  ::kill(-target, sig);
  ::kill(target, sig);
}
#endif

bool IsPythonScriptPath(const QString& path) {
  const QString cleanPath = path.trimmed();
  return cleanPath.endsWith(".py", Qt::CaseInsensitive);
}

QString ShellEscape(const QString& value) {
  QString escaped = value;
  escaped.replace('\'', "'\"'\"'");
  return "'" + escaped + "'";
}

QString BuildShellCommand(const QString& program,
                          const QStringList& arguments) {
  QStringList commandParts;
  commandParts << ShellEscape(program);
  for (const QString& argument : arguments) {
    commandParts << ShellEscape(argument);
  }
  return commandParts.join(' ');
}

QString ResolveBashExecutable() {
  const QString bash = QStandardPaths::findExecutable("bash");
  if (!bash.isEmpty()) {
    return bash;
  }

  const QFileInfo fallbackBash("/bin/bash");
  if (fallbackBash.exists() && fallbackBash.isExecutable()) {
    return fallbackBash.absoluteFilePath();
  }

  return QString();
}

QString ResolvePythonInterpreter() {
  const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  const QString configuredPython = env.value("OPTICSBENCHUI_PYTHON").trimmed();
  if (!configuredPython.isEmpty()) {
    return configuredPython;
  }

  const QString python = QStandardPaths::findExecutable("python");
  if (!python.isEmpty()) {
    return python;
  }

  const QString python3 = QStandardPaths::findExecutable("python3");
  if (!python3.isEmpty()) {
    return python3;
  }

#ifdef Q_OS_WIN
  const QString pyLauncher = QStandardPaths::findExecutable("py");
  if (!pyLauncher.isEmpty()) {
    return pyLauncher;
  }
#endif
  return QString();
}
}  // namespace

AnalysisThread::AnalysisThread(QObject* parent)
    : QThread(parent),
      tasks(),
      mutex(new QMutex()),
      suspend(true),
      currentPid(0) {}

AnalysisThread::~AnalysisThread() {
  QLOG_DEBUG() << "Deleting AnalysisThread";
  stop();
  wait(2000);
  delete mutex;
}

void AnalysisThread::setTasks(const QVector<AnalysisTask>& tasks) {
  this->tasks = tasks;
}

void AnalysisThread::stop() {
  QMutexLocker locker(mutex);
  suspend = true;
#ifdef Q_OS_UNIX
  if (currentPid > 0) {
    KillProcessTree(currentPid, SIGTERM);
    if (IsProcessAlive(currentPid)) {
      KillProcessTree(currentPid, SIGKILL);
    }
  }
#endif
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

    QString program = task.codePath.trimmed();
    QStringList arguments = splitArguments(task.arguments);
    bool runInLoginShell = false;
    if (program.contains(' ')) {
      const QStringList commandParts = splitArguments(program);
      if (!commandParts.isEmpty()) {
        program = commandParts.first();
        QStringList extraArgs = commandParts;
        extraArgs.removeFirst();
        if (!extraArgs.isEmpty()) {
          arguments = extraArgs + arguments;
        }
      }
    }

    // Run Python scripts through an explicit interpreter so dependencies
    // resolve from the expected Python environment.
    if (IsPythonScriptPath(program)) {
      runInLoginShell = true;
      const QString pythonInterpreter = ResolvePythonInterpreter();
      if (!pythonInterpreter.isEmpty()) {
        arguments.prepend(program);
#ifdef Q_OS_WIN
        const QString pyName =
            QFileInfo(pythonInterpreter).fileName().toLower();
        if (pyName == "py" || pyName == "py.exe") {
          arguments.prepend("-3");
        }
#endif
        program = pythonInterpreter;
      } else {
        emit showWarning(
            "No Python interpreter found in PATH. Trying direct script run.");
      }
    }

    QString launchProgram = program;
    QStringList launchArguments = arguments;
#ifdef Q_OS_UNIX
    if (runInLoginShell) {
      const QString bash = ResolveBashExecutable();
      if (!bash.isEmpty()) {
        const QString command = BuildShellCommand(program, arguments);
        launchProgram = bash;
        launchArguments =
            QStringList() << "-lc"
                          << QString("if [ -f \"$HOME/.bashrc\" ]; then "
                                     ". \"$HOME/.bashrc\" >/dev/null 2>&1; "
                                     "fi; %1")
                                 .arg(command);
      } else {
        emit showWarning("bash not found; running analysis without shell init.");
      }
    }
#endif

    QProcess localProcess;
    localProcess.setProcessChannelMode(QProcess::MergedChannels);
    localProcess.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    localProcess.start(launchProgram, launchArguments);

    if (!localProcess.waitForStarted()) {
      const QString output =
          "Unable to start command.\n"
          "Command: " + launchProgram + "\n"
          "Arguments: " + launchArguments.join(' ') + "\n"
          "Error: " + localProcess.errorString();
      emit analysisFinished(task.record, false, output);
      emit showWarning("Unable to start analysis command: " + task.codePath);
      if (currentPid != 0) {
        currentPid = 0;
        emit pidChanged(0);
      }
      continue;
    }
    currentPid = static_cast<qint64>(localProcess.processId());
#ifdef Q_OS_UNIX
    if (currentPid > 0) {
      setpgid(static_cast<pid_t>(currentPid),
              static_cast<pid_t>(currentPid));
    }
#endif
    emit pidChanged(currentPid);

    bool stopped = false;
    QString outputBuffer;
    while (!localProcess.waitForFinished(100)) {
      const QString chunk = localProcess.readAllStandardOutput();
      if (!chunk.isEmpty()) {
        outputBuffer += chunk;
        emit analysisOutput(task.record, chunk);
      }
      QMutexLocker locker(mutex);
      if (suspend) {
        stopped = true;
        break;
      }
    }

    if (stopped) {
      localProcess.terminate();
      localProcess.waitForFinished(2000);
      if (localProcess.state() != QProcess::NotRunning) {
        localProcess.kill();
        localProcess.waitForFinished(2000);
      }
#ifdef Q_OS_UNIX
      if (IsProcessAlive(currentPid)) {
        KillProcessTree(currentPid, SIGKILL);
      }
#endif
      const QString chunk = localProcess.readAllStandardOutput();
      if (!chunk.isEmpty()) {
        outputBuffer += chunk;
        emit analysisOutput(task.record, chunk);
      }
      emit analysisFinished(task.record, false, "Stopped by user.");
      currentPid = 0;
      emit pidChanged(0);
      break;
    }

    const QString remaining = localProcess.readAllStandardOutput();
    if (!remaining.isEmpty()) {
      outputBuffer += remaining;
      emit analysisOutput(task.record, remaining);
    }

    const bool success =
        (localProcess.exitStatus() == QProcess::NormalExit &&
         localProcess.exitCode() == 0);
    const QString output =
        "Exit code: " + QString::number(localProcess.exitCode());
    emit analysisFinished(task.record, success, output);
    currentPid = 0;
    emit pidChanged(0);
  }

  QMutexLocker locker(mutex);
  suspend = true;
  if (currentPid != 0) {
    currentPid = 0;
    emit pidChanged(0);
  }
  QLOG_DEBUG() << "AnalysisThread::run> End of thread";
}
