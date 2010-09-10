#ifndef MAIN_H
#define MAIN_H

class CardDatabase;
class QTranslator;

extern CardDatabase *db;

extern QTranslator *translator;
const QString translationPrefix = "cockatrice";

void installNewTranslator();

#endif
