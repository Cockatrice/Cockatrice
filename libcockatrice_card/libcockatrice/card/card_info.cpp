#include "card_info.h"

#include "game_specific_terms.h"
#include "printing/printing_info.h"
#include "relation/card_relation.h"
#include "set/card_set.h"

#include <QDir>
#include <QRegularExpression>
#include <QSharedPointer>
#include <QString>
#include <QVariant>
#include <algorithm>
#include <libcockatrice/settings/cache_settings.h>
#include <utility>

class CardRelation;
class CardSet;
class CardInfo;

using CardInfoPtr = QSharedPointer<CardInfo>;

CardInfo::CardInfo(const QString &_name,
                   const QString &_text,
                   bool _isToken,
                   QVariantHash _properties,
                   const QList<CardRelation *> &_relatedCards,
                   const QList<CardRelation *> &_reverseRelatedCards,
                   SetToPrintingsMap _sets,
                   bool _cipt,
                   bool _landscapeOrientation,
                   int _tableRow,
                   bool _upsideDownArt)
    : name(_name), text(_text), isToken(_isToken), properties(std::move(_properties)), relatedCards(_relatedCards),
      reverseRelatedCards(_reverseRelatedCards), setsToPrintings(std::move(_sets)), cipt(_cipt),
      landscapeOrientation(_landscapeOrientation), tableRow(_tableRow), upsideDownArt(_upsideDownArt)
{
    simpleName = CardInfo::simplifyName(name);

    refreshCachedSetNames();
}

CardInfoPtr CardInfo::newInstance(const QString &_name)
{
    return newInstance(_name, QString(), false, QVariantHash(), QList<CardRelation *>(), QList<CardRelation *>(),
                       SetToPrintingsMap(), false, false, 0, false);
}

CardInfoPtr CardInfo::newInstance(const QString &_name,
                                  const QString &_text,
                                  bool _isToken,
                                  QVariantHash _properties,
                                  const QList<CardRelation *> &_relatedCards,
                                  const QList<CardRelation *> &_reverseRelatedCards,
                                  SetToPrintingsMap _sets,
                                  bool _cipt,
                                  bool _landscapeOrientation,
                                  int _tableRow,
                                  bool _upsideDownArt)
{
    CardInfoPtr ptr(new CardInfo(_name, _text, _isToken, std::move(_properties), _relatedCards, _reverseRelatedCards,
                                 _sets, _cipt, _landscapeOrientation, _tableRow, _upsideDownArt));
    ptr->setSmartPointer(ptr);

    for (const auto &printings : _sets) {
        for (const PrintingInfo &printing : printings) {
            printing.getSet()->append(ptr);
            break;
        }
    }

    return ptr;
}

QString CardInfo::getCorrectedName() const
{
    // remove all the characters reserved in windows file paths,
    // other oses only disallow a subset of these so it covers all
    static const QRegularExpression rmrx(R"(( // |[*<>:"\\?\x00-\x08\x10-\x1f]))");
    static const QRegularExpression spacerx(R"([/\x09-\x0f])");
    static const QString space(' ');
    QString result = name;
    // Fire // Ice, Circle of Protection: Red, "Ach! Hans, Run!", Who/What/When/Where/Why, Question Elemental?
    return result.remove(rmrx).replace(spacerx, space);
}

void CardInfo::addToSet(const CardSetPtr &_set, const PrintingInfo _info)
{
    if (!_set->contains(smartThis)) {
        _set->append(smartThis);
    }
    if (!setsToPrintings[_set->getShortName()].contains(_info)) {
        setsToPrintings[_set->getShortName()].append(_info);
    }

    refreshCachedSetNames();
}

void CardInfo::combineLegalities(const QVariantHash &props)
{
    QHashIterator<QString, QVariant> it(props);
    while (it.hasNext()) {
        it.next();
        if (it.key().startsWith("format-")) {
            smartThis->setProperty(it.key(), it.value().toString());
        }
    }
}

void CardInfo::refreshCachedSetNames()
{
    QStringList setList;
    // update the cached list of set names
    for (const auto &printings : setsToPrintings) {
        for (const auto &printing : printings) {
            if (printing.getSet()->getEnabled()) {
                setList << printing.getSet()->getShortName();
            }
            break;
        }
    }
    setsNames = setList.join(", ");
}

QString CardInfo::simplifyName(const QString &name)
{
    static const QRegularExpression spaceOrSplit("(\\s+|\\/\\/.*)");
    static const QRegularExpression nonAlnum("[^a-z0-9]");

    QString simpleName = name.toLower();

    // remove spaces and right halves of split cards
    simpleName.remove(spaceOrSplit);

    // So Aetherling would work, but not Ætherling since 'Æ' would get replaced
    // with nothing.
    simpleName.replace("æ", "ae");

    // Replace Jötun Grunt with Jotun Grunt.
    simpleName = simpleName.normalized(QString::NormalizationForm_KD);

    // remove all non alphanumeric characters from the name
    simpleName.remove(nonAlnum);
    return simpleName;
}

const QChar CardInfo::getColorChar() const
{
    QString colors = getColors();
    switch (colors.size()) {
        case 0:
            return QChar();
        case 1:
            return colors.at(0);
        default:
            return QChar('m');
    }
}

void CardInfo::resetReverseRelatedCards2Me()
{
    for (CardRelation *cardRelation : this->getReverseRelatedCards2Me()) {
        cardRelation->deleteLater();
    }
    reverseRelatedCardsToMe = QList<CardRelation *>();
}

// Back-compatibility methods. Remove ASAP
const QString CardInfo::getCardType() const
{
    return getProperty(Mtg::CardType);
}
void CardInfo::setCardType(const QString &value)
{
    setProperty(Mtg::CardType, value);
}
const QString CardInfo::getCmc() const
{
    return getProperty(Mtg::ConvertedManaCost);
}
const QString CardInfo::getColors() const
{
    return getProperty(Mtg::Colors);
}
void CardInfo::setColors(const QString &value)
{
    setProperty(Mtg::Colors, value);
}
const QString CardInfo::getLoyalty() const
{
    return getProperty(Mtg::Loyalty);
}
const QString CardInfo::getMainCardType() const
{
    return getProperty(Mtg::MainCardType);
}
const QString CardInfo::getManaCost() const
{
    return getProperty(Mtg::ManaCost);
}
const QString CardInfo::getPowTough() const
{
    return getProperty(Mtg::PowTough);
}
void CardInfo::setPowTough(const QString &value)
{
    setProperty(Mtg::PowTough, value);
}
