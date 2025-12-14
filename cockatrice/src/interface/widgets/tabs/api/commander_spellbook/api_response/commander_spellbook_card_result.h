#ifndef COCKATRICE_COMMANDER_SPELLBOOK_CARD_RESULT_H
#define COCKATRICE_COMMANDER_SPELLBOOK_CARD_RESULT_H
#include <QJsonObject>
#include <QString>

class CommanderSpellbookCardResult
{
public:
    void fromJson(const QJsonObject &json);

    QString id;
    QString name;
    QString oracleId;
    bool spoiler = false;
    QString typeLine;

    QString imageUriFrontPng;
    QString imageUriFrontLarge;
    QString imageUriFrontNormal;
    QString imageUriFrontSmall;
    QString imageUriFrontArtCrop;

    QString imageUriBackPng;
    QString imageUriBackLarge;
    QString imageUriBackNormal;
    QString imageUriBackSmall;
    QString imageUriBackArtCrop;
};

#endif // COCKATRICE_COMMANDER_SPELLBOOK_CARD_RESULT_H
