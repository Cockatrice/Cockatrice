#ifndef COCKATRICE_EMAILPARSER_H
#define COCKATRICE_EMAILPARSER_H

#include <QPair>
#include <QString>

class EmailParser
{
public:
    static QPair<QString, QString> parseEmailAddress(const QString &dirtyEmailAddress);
    static QString getParsedEmailAddress(const QString &dirtyEmailAddress);
    static QString getParsedEmailAddress(const QPair<QString, QString> &emailAddressIntermediate);
};

#endif // COCKATRICE_EMAILPARSER_H
