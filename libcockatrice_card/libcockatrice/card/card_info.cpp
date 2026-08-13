#include "card_info.h"

#include "game_specific_terms.h"
#include "printing/printing_info.h"
#include "relation/card_relation.h"
#include "set/card_set.h"

#include <QDir>
#include <QRegularExpression>
#include <QSharedPointer>
#include <QString>
#include <algorithm>
#include <utility>

class CardRelation;
class CardSet;
class CardInfo;

using CardInfoPtr = QSharedPointer<CardInfo>;

const QHash<QString, QString> &CardInfo::getPropertiesHash() const
{
    return properties.getProperties();
}

void CardInfo::setProperty(const QString &_name, const QString &_value)
{
    bool changed = properties.insert(_name, _value);
    if (!changed) {
        return;
    }

    emit cardInfoChanged(smartThis);
}

CardInfo::CardInfo(const QString &_name,
                   const QString &_text,
                   bool _isToken,
                   const QHash<QString, QString> &_properties,
                   const QList<CardRelation *> &_relatedCards,
                   const QList<CardRelation *> &_reverseRelatedCards,
                   SetToPrintingsMap _sets,
                   const UiAttributes _uiAttributes)
    : name(_name), text(_text), isToken(_isToken), properties(LazyPropertiesHash(_properties)),
      relatedCards(_relatedCards), reverseRelatedCards(_reverseRelatedCards), setsToPrintings(std::move(_sets)),
      uiAttributes(_uiAttributes)
{
    simpleName = CardInfo::simplifyName(name);

    refreshCachedSets();
}

CardInfo::CardInfo(const QString &_name,
                   const QString &_text,
                   bool _isToken,
                   const QByteArray &_propertiesBlob,
                   const QList<CardRelation *> &_relatedCards,
                   const QList<CardRelation *> &_reverseRelatedCards,
                   SetToPrintingsMap _sets,
                   const UiAttributes _uiAttributes,
                   QString _simpleName,
                   QSet<QString> _altNames)
    : name(_name), simpleName(std::move(_simpleName)), text(_text), isToken(_isToken),
      properties(LazyPropertiesHash(_propertiesBlob)), relatedCards(_relatedCards),
      reverseRelatedCards(_reverseRelatedCards), setsToPrintings(std::move(_sets)), uiAttributes(_uiAttributes),
      altNames(std::move(_altNames))
{
    // propertiesBlob is materialized lazily on first query; simpleName and
    // altNames are supplied by the caller (binary cache). Only the set-name
    // display list still depends on the current enabled-set state.
    refreshCachedSetNames();
}

CardInfoPtr CardInfo::newInstance(const QString &_name)
{
    return newInstance(_name, "", false, {}, {}, {}, {}, {});
}

CardInfoPtr CardInfo::newInstance(const QString &_name,
                                  const QString &_text,
                                  bool _isToken,
                                  const QHash<QString, QString> &_properties,
                                  const QList<CardRelation *> &_relatedCards,
                                  const QList<CardRelation *> &_reverseRelatedCards,
                                  SetToPrintingsMap _sets,
                                  const UiAttributes _uiAttributes)
{
    CardInfoPtr ptr(
        new CardInfo(_name, _text, _isToken, _properties, _relatedCards, _reverseRelatedCards, _sets, _uiAttributes));
    ptr->setSmartPointer(ptr);

    for (const auto &printings : _sets) {
        for (const PrintingInfo &printing : printings) {
            printing.getSet()->append(ptr);
            break;
        }
    }

    return ptr;
}

CardInfoPtr CardInfo::newInstance(const QString &_name,
                                  const QString &_text,
                                  bool _isToken,
                                  QByteArray _propertiesBlob,
                                  const QList<CardRelation *> &_relatedCards,
                                  const QList<CardRelation *> &_reverseRelatedCards,
                                  SetToPrintingsMap _sets,
                                  const UiAttributes _uiAttributes,
                                  QString _simpleName,
                                  QSet<QString> _altNames,
                                  bool _appendToSets)
{
    CardInfoPtr ptr(new CardInfo(_name, _text, _isToken, std::move(_propertiesBlob), _relatedCards,
                                 _reverseRelatedCards, _sets, _uiAttributes, std::move(_simpleName),
                                 std::move(_altNames)));
    ptr->setSmartPointer(ptr);

    if (_appendToSets) {
        for (const auto &printings : _sets) {
            for (const PrintingInfo &printing : printings) {
                printing.getSet()->append(ptr);
                break;
            }
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

QString CardInfo::getLegalityProp(const QString &format) const
{
    return getProperty("format-" + format);
}

bool CardInfo::isLegalInFormat(const QString &format) const
{
    if (format.isEmpty()) {
        return true;
    }

    QString formatLegality = getLegalityProp(format);
    return formatLegality == "legal" || formatLegality == "restricted";
}

void CardInfo::addToSet(const CardSetPtr &_set, const PrintingInfo &_info)
{
    if (!_set->contains(smartThis)) {
        _set->append(smartThis);
    }
    if (!setsToPrintings[_set->getShortName()].contains(_info)) {
        setsToPrintings[_set->getShortName()].append(_info);
    }

    refreshCachedSets();
}

void CardInfo::combineLegalities(const QHash<QString, QString> &props)
{
    QHashIterator it(props);
    while (it.hasNext()) {
        it.next();
        if (it.key().startsWith("format-")) {
            properties.insert(it.key(), it.value());
        }
    }
    emit cardInfoChanged(smartThis);
}

void CardInfo::refreshCachedSets()
{
    refreshCachedSetNames();
    refreshCachedAltNames();
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

void CardInfo::refreshCachedAltNames()
{
    altNames.clear();

    // update the altNames with the flavorNames
    for (const auto &printings : setsToPrintings) {
        for (const auto &printing : printings) {
            QString flavorName = printing.getFlavorName();
            if (!flavorName.isEmpty()) {
                altNames.insert(flavorName);
            }
        }
    }
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

QChar CardInfo::getColorChar() const
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
QString CardInfo::getCardType() const
{
    return getProperty(Mtg::CardType);
}
void CardInfo::setCardType(const QString &value)
{
    setProperty(Mtg::CardType, value);
}
QString CardInfo::getCmc() const
{
    return getProperty(Mtg::ConvertedManaCost);
}
QString CardInfo::getColors() const
{
    return getProperty(Mtg::Colors);
}
void CardInfo::setColors(const QString &value)
{
    setProperty(Mtg::Colors, value);
}
QString CardInfo::getLoyalty() const
{
    return getProperty(Mtg::Loyalty);
}
QString CardInfo::getMainCardType() const
{
    return getProperty(Mtg::MainCardType);
}
QString CardInfo::getManaCost() const
{
    return getProperty(Mtg::ManaCost);
}
QString CardInfo::getPowTough() const
{
    return getProperty(Mtg::PowTough);
}
void CardInfo::setPowTough(const QString &value)
{
    setProperty(Mtg::PowTough, value);
}
