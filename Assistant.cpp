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

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QLibraryInfo>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QStandardPaths>
#include <QtCore/QProcess>

#include <QMessageBox>

#include "Assistant.h"

Assistant::Assistant(QString appDirPath)
    : proc(nullptr),
      appDirPath(appDirPath) {}

Assistant::~Assistant() {
  if (proc && proc->state() == QProcess::Running) {
    proc->terminate();
    proc->waitForFinished(3000);
  }
  delete proc;
}

void Assistant::showDocumentation(const QString &page) {
  if (!startAssistant()) {
    return;
  }

  proc->write(page.toLocal8Bit() + '\n');
}

bool Assistant::startAssistant() {
  if (!proc) {
    proc = new QProcess();
  }

  if (proc->state() != QProcess::Running) {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("QT_STYLE_OVERRIDE", "Fusion");
    proc->setProcessEnvironment(env);

    QString app = QLibraryInfo::location(QLibraryInfo::BinariesPath) + QDir::separator();
#if !defined(Q_OS_MAC)
    app += QLatin1String("assistant");
#else
    app += QLatin1String("Assistant.app/Contents/MacOS/Assistant");
#endif
    if (!QFile::exists(app)) {
      const QString fallback = QStandardPaths::findExecutable("assistant");
      const QString fallbackQt5 = QStandardPaths::findExecutable("assistant-qt5");
      const QString fallbackQt = QStandardPaths::findExecutable("qtassistant");
      if (!fallback.isEmpty()) {
        app = fallback;
      } else if (!fallbackQt5.isEmpty()) {
        app = fallbackQt5;
      } else if (!fallbackQt.isEmpty()) {
        app = fallbackQt;
      }
    }
    QStringList args;
    const QString qssPath =
        appDirPath + QLatin1String("/docs/help/assistant-dark.qss");
    if (QFile::exists(qssPath)) {
      args << QLatin1String("-stylesheet") << qssPath;
    }
    args << QLatin1String("-collectionFile")
         << appDirPath + QLatin1String("/docs/help/OpticsBenchUIColl.qhc")
         << QLatin1String("-enableRemoteControl");
    proc->start(app, args);
    if (!proc->waitForStarted()) {
      QMessageBox::critical(0, QObject::tr("OpticsBenchUI"),
                            QObject::tr("Unable to launch Qt Assistant (%1)").arg(app));
      return false;
    }
  }
  return true;
}
