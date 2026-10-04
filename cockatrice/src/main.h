/**
 * @file main.h
 * @ingroup Core
 */
//! \todo Document this file.

#ifndef MAIN_H
#define MAIN_H

#include <QLoggingCategory>

inline Q_LOGGING_CATEGORY(MainLog, "main");

class CardDatabase;
class QString;
class QSystemTrayIcon;
class QTranslator;

extern CardDatabase *db;

extern QSystemTrayIcon *trayIcon;
extern QTranslator *translator;
extern const QString translationPrefix;
extern QString translationPath;

void installNewTranslator();

QString const generateClientID();

#endif
