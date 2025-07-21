#include "card_info.h"

#include "../../client/ui/picture_loader/picture_loader.h"
#include "../../settings/cache_settings.h"
#include "../game_specific_terms.h"

#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QRegularExpression>
#include <algorithm>
#include <utility>

const char *CardSet::TOKENS_SETNAME = "TK";

CardSet::CardSet(const QString &_shortName,
                 const QString &_longName,
                 const QString &_setType,
                 const QDate &_releaseDate,
                 const CardSet::Priority _priority)
    : shortName(_shortName), longName(_longName), releaseDate(_releaseDate), setType(_setType), priority(_priority)
{
    loadSetOptions();
}

CardSetPtr CardSet::newInstance(const QString &_shortName,
                                const QString &_longName,
                                const QString &_setType,
                                const QDate &_releaseDate,
                                const Priority _priority)
{
    CardSetPtr ptr(new CardSet(_shortName, _longName, _setType, _releaseDate, _priority));
    // ptr->setSmartPointer(ptr);
    return ptr;
}

QString CardSet::getCorrectedShortName() const
{
    // For Windows machines.
    QSet<QString> invalidFileNames;
    invalidFileNames << "CON"
                     << "PRN"
                     << "AUX"
                     << "NUL"
                     << "COM1"
                     << "COM2"
                     << "COM3"
                     << "COM4"
                     << "COM5"
                     << "COM6"
                     << "COM7"
                     << "COM8"
                     << "COM9"
                     << "LPT1"
                     << "LPT2"
                     << "LPT3"
                     << "LPT4"
                     << "LPT5"
                     << "LPT6"
                     << "LPT7"
                     << "LPT8"
                     << "LPT9";

    return invalidFileNames.contains(shortName) ? shortName + "_" : shortName;
}

void CardSet::loadSetOptions()
{
    sortKey = SettingsCache::instance().cardDatabase().getSortKey(shortName);
    enabled = SettingsCache::instance().cardDatabase().isEnabled(shortName);
    isknown = SettingsCache::instance().cardDatabase().isKnown(shortName);
}

void CardSet::setSortKey(unsigned int _sortKey)
{
    sortKey = _sortKey;
    SettingsCache::instance().cardDatabase().setSortKey(shortName, _sortKey);
}

void CardSet::setEnabled(bool _enabled)
{
    enabled = _enabled;
    SettingsCache::instance().cardDatabase().setEnabled(shortName, _enabled);
}

void CardSet::setIsKnown(bool _isknown)
{
    isknown = _isknown;
    SettingsCache::instance().cardDatabase().setIsKnown(shortName, _isknown);
}

class SetList::KeyCompareFunctor
{
public:
    inline bool operator()(const CardSetPtr &a, const CardSetPtr &b) const
    {
        if (a.isNull() || b.isNull()) {
            qCWarning(CardInfoLog) << "SetList::KeyCompareFunctor a or b is null";
            return false;
        }

        return a->getSortKey() < b->getSortKey();
    }
};

void SetList::sortByKey()
{
    std::sort(begin(), end(), KeyCompareFunctor());
}

int SetList::getEnabledSetsNum()
{
    int num = 0;
    for (int i = 0; i < size(); ++i) {
        CardSetPtr set = at(i);
        if (set && set->getEnabled()) {
            ++num;
        }
    }
    return num;
}

int SetList::getUnknownSetsNum()
{
    int num = 0;
    for (int i = 0; i < size(); ++i) {
        CardSetPtr set = at(i);
        if (set && !set->getIsKnown() && !set->getIsKnownIgnored()) {
            ++num;
        }
    }
    return num;
}

QStringList SetList::getUnknownSetsNames()
{
    QStringList sets = QStringList();
    for (int i = 0; i < size(); ++i) {
        CardSetPtr set = at(i);
        if (set && !set->getIsKnown() && !set->getIsKnownIgnored()) {
            sets << set->getShortName();
        }
    }
    return sets;
}

void SetList::enableAllUnknown()
{
    for (int i = 0; i < size(); ++i) {
        CardSetPtr set = at(i);
        if (set && !set->getIsKnown() && !set->getIsKnownIgnored()) {
            set->setIsKnown(true);
            set->setEnabled(true);
        } else if (set && set->getIsKnownIgnored() && !set->getEnabled()) {
            set->setEnabled(true);
        }
    }
}

void SetList::enableAll()
{
    for (int i = 0; i < size(); ++i) {
        CardSetPtr set = at(i);

        if (set == nullptr) {
            qCWarning(CardInfoLog) << "enabledAll has null";
            continue;
        }

        if (!set->getIsKnownIgnored()) {
            set->setIsKnown(true);
        }

        set->setEnabled(true);
    }
}

void SetList::markAllAsKnown()
{
    for (int i = 0; i < size(); ++i) {
        CardSetPtr set = at(i);
        if (set && !set->getIsKnown() && !set->getIsKnownIgnored()) {
            set->setIsKnown(true);
            set->setEnabled(false);
        } else if (set && set->getIsKnownIgnored() && !set->getEnabled()) {
            set->setEnabled(true);
        }
    }
}

void SetList::guessSortKeys()
{
    defaultSort();
    for (int i = 0; i < size(); ++i) {
        CardSetPtr set = at(i);
        if (set.isNull()) {
            qCWarning(CardInfoLog) << "guessSortKeys set is null";
            continue;
        }
        set->setSortKey(i);
    }
}

void SetList::defaultSort()
{
    std::sort(begin(), end(), [](const CardSetPtr &a, const CardSetPtr &b) {
        // Sort by priority, then by release date, then by short name
        if (a->getPriority() != b->getPriority()) {
            return a->getPriority() < b->getPriority(); // lowest first
        } else if (a->getReleaseDate() != b->getReleaseDate()) {
            return a->getReleaseDate() > b->getReleaseDate(); // most recent first
        } else {
            return a->getShortName() < b->getShortName(); // alphabetically
        }
    });
}

PrintingInfo::PrintingInfo(const CardSetPtr &_set) : set(_set)
{
}

/**
 * Gets the uuid property of the printing, or an empty string if the property isn't present
 */
QString PrintingInfo::getUuid() const
{
    return properties.value("uuid").toString();
}

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

CardRelation::CardRelation(const QString &_name,
                           AttachType _attachType,
                           bool _isCreateAllExclusion,
                           bool _isVariableCount,
                           int _defaultCount,
                           bool _isPersistent)
    : name(_name), attachType(_attachType), isCreateAllExclusion(_isCreateAllExclusion),
      isVariableCount(_isVariableCount), defaultCount(_defaultCount), isPersistent(_isPersistent)
{
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
