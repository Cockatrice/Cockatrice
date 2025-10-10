/**
 * @file main.h
 * @ingroup Core
 * @brief TODO: Document this.
 */

#ifndef MAIN_H
#define MAIN_H

#include <QLoggingCategory>
#include <libcockatrice/utility/macros.h>

inline Q_LOGGING_CATEGORY(MainLog, "main");
inline Q_LOGGING_CATEGORY(QtTranslatorDebug, "qt_translator");

class CardDatabase;
class QString;
class QSystemTrayIcon;
class QTranslator;
class SoundEngine;

extern CardDatabase *db;

extern QSystemTrayIcon *trayIcon;
extern QTranslator *translator;
extern const QString translationPrefix;
extern QString translationPath;

void installNewTranslator();

QString const generateClientID();

#endif
