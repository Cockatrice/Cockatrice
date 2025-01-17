#ifndef MAIN_H
#define MAIN_H

#include "utility/macros.h"

#include <QLoggingCategory>

inline Q_LOGGING_CATEGORY(QtTranslatorDebug, "qt_translator.debug");

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
