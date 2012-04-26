#ifndef MAIN_H
#define MAIN_H

class CardDatabase;
class QTranslator;
class SoundEngine;

extern CardDatabase *db;

extern QTranslator *translator;
extern const QString translationPrefix;
extern QString translationPath;

void installNewTranslator();

#endif
