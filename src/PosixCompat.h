#ifndef POSIXCOMPAT_H
#define POSIXCOMPAT_H

#include <QtGlobal>

#ifdef Q_OS_WIN
#include <QThread>

inline int usleep(unsigned int usec) {
  QThread::usleep(usec);
  return 0;
}

inline unsigned int sleep(unsigned int sec) {
  QThread::sleep(sec);
  return 0;
}

#else
#include <unistd.h>
#endif

#endif  // POSIXCOMPAT_H
