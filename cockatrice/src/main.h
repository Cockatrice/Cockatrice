#ifndef MAIN_H
#define MAIN_H

class CardDatabase;
class QTranslator;
class QSystemTrayIcon;
class SoundEngine;

extern CardDatabase *db;
extern QSystemTrayIcon *trayIcon;
extern QTranslator *translator;
extern const QString translationPrefix;
extern QString translationPath;

void installNewTranslator();

QString const generateClientID();

#endif
