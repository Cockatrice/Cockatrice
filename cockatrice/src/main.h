#ifndef MAIN_H
#define MAIN_H

class CardDatabase;
class QTranslator;

extern CardDatabase *db;

extern QTranslator *translator;
const QString translationPrefix = "cockatrice";
const QString versionString = "0.20110303";

void installNewTranslator();

#endif
