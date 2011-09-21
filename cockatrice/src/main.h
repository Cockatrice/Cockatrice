#ifndef MAIN_H
#define MAIN_H

class CardDatabase;
class QTranslator;
class SoundEngine;

extern CardDatabase *db;

extern QTranslator *translator;
const QString translationPrefix = "cockatrice";
const QString versionString = "0.20110921";

void installNewTranslator();

#endif
