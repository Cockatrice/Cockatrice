#ifndef MAIN_H
#define MAIN_H

class QTranslator;

extern QTranslator *translator;
extern const QString translationPrefix;
extern QString translationPath;

void installNewTranslator();

#endif
