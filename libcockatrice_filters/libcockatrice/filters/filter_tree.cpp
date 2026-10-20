#include "filter_tree.h"

#include "filter_card.h"
#include "libcockatrice/card/card_localization.h"
#include "libcockatrice/card/printing/printing_info.h"
#include "libcockatrice/card/set/card_set.h"

#include <QChar>
#include <QList>
#include <QSharedPointer>
#include <QStringList>
#include <QStringView>
#include <algorithm>
#include <qnamespace.h>

namespace
{
/** Separator between the main and sub types of a card type ("Creature — Elf"). */
const QString &mainSubTypeSeparator()
{
    static const QString separator = QStringLiteral(" — ");
    return separator;
}
} // namespace

template <class T> FilterTreeNode *FilterTreeBranch<T>::nodeAt(int i) const
{
    return (childNodes.size() > i) ? childNodes.at(i) : nullptr;
}

template <class T> void FilterTreeBranch<T>::deleteAt(int i)
{
    preRemoveChild(this, i);
    delete childNodes.takeAt(i);
    postRemoveChild(this, i);
    nodeChanged();
}

template <class T> int FilterTreeBranch<T>::childIndex(const FilterTreeNode *node) const
{
    auto *unconst = const_cast<FilterTreeNode *>(node);
    auto downcasted = dynamic_cast<T>(unconst);
    return (downcasted) ? childNodes.indexOf(downcasted) : -1;
}

template <class T> FilterTreeBranch<T>::~FilterTreeBranch()
{
    while (!childNodes.isEmpty()) {
        delete childNodes.takeFirst();
    }
}

const FilterItemList *LogicMap::findTypeList(CardFilter::Type type) const
{
    QList<FilterItemList *>::const_iterator i;

    for (i = childNodes.constBegin(); i != childNodes.constEnd(); ++i) {
        if ((*i)->type == type) {
            return *i;
        }
    }

    return nullptr;
}

FilterItemList *LogicMap::typeList(CardFilter::Type type)
{
    QList<FilterItemList *>::iterator i;
    int count = 0;

    for (i = childNodes.begin(); i != childNodes.end(); ++i) {
        if ((*i)->type == type) {
            break;
        }
        count++;
    }

    if (i == childNodes.end()) {
        preInsertChild(this, count);
        i = childNodes.insert(i, new FilterItemList(type, this));
        postInsertChild(this, count);
        nodeChanged();
    }

    return *i;
}

FilterTreeNode *LogicMap::parent() const
{
    return p;
}

int FilterItemList::termIndex(const QString &term) const
{
    for (int i = 0; i < childNodes.count(); i++) {
        if ((childNodes.at(i))->term == term) {
            return i;
        }
    }

    return -1;
}

FilterTreeNode *FilterItemList::termNode(const QString &term)
{
    int i = termIndex(term);
    if (i < 0) {
        FilterItem *fi = new FilterItem(term, this);
        int count = childNodes.count();

        preInsertChild(this, count);
        childNodes.append(fi);
        postInsertChild(this, count);
        nodeChanged();

        return fi;
    }

    return childNodes.at(i);
}

bool FilterItemList::testTypeAnd(const CardInfoPtr &info,
                                 CardFilter::Attr attr,
                                 const CardSearchLanguage &searchLanguage) const
{
    for (auto i = childNodes.constBegin(); i != childNodes.constEnd(); i++) {
        if (!(*i)->isEnabled()) {
            continue;
        }

        if (!(*i)->acceptCardAttr(info, attr, searchLanguage)) {
            return false;
        }
    }

    return true;
}

bool FilterItemList::testTypeAndNot(const CardInfoPtr &info,
                                    CardFilter::Attr attr,
                                    const CardSearchLanguage &searchLanguage) const
{
    // if any one in the list is true, return false
    return !testTypeOr(info, attr, searchLanguage);
}

bool FilterItemList::testTypeOr(const CardInfoPtr &info,
                                CardFilter::Attr attr,
                                const CardSearchLanguage &searchLanguage) const
{
    bool noChildEnabledChild = true;

    for (auto i = childNodes.constBegin(); i != childNodes.constEnd(); i++) {
        if (!(*i)->isEnabled()) {
            continue;
        }

        if (noChildEnabledChild) {
            noChildEnabledChild = false;
        }

        if ((*i)->acceptCardAttr(info, attr, searchLanguage)) {
            return true;
        }
    }

    return noChildEnabledChild;
}

bool FilterItemList::testTypeOrNot(const CardInfoPtr &info,
                                   CardFilter::Attr attr,
                                   const CardSearchLanguage &searchLanguage) const
{
    // if any one in the list is false, return true
    return !testTypeAnd(info, attr, searchLanguage);
}

FilterItem::FilterItem(QString trm, FilterItemList *parent) : p(parent), term(std::move(trm))
{
    lowercaseTerm = term.toLower();

    switch (attr()) {
        case CardFilter::AttrColor: {
            QString converted = term.trimmed();

            converted.replace("green", "g", Qt::CaseInsensitive);
            converted.replace("grn", "g", Qt::CaseInsensitive);
            converted.replace("blue", "u", Qt::CaseInsensitive);
            converted.replace("blu", "u", Qt::CaseInsensitive);
            converted.replace("black", "b", Qt::CaseInsensitive);
            converted.replace("blk", "b", Qt::CaseInsensitive);
            converted.replace("red", "r", Qt::CaseInsensitive);
            converted.replace("white", "w", Qt::CaseInsensitive);
            converted.replace("wht", "w", Qt::CaseInsensitive);
            converted.replace("colorless", "c", Qt::CaseInsensitive);
            converted.replace("colourless", "c", Qt::CaseInsensitive);
            converted.replace("none", "c", Qt::CaseInsensitive);

            converted.replace(QString(" "), QString(""), Qt::CaseInsensitive);

            convertedColorTerm = converted;
            break;
        }
        case CardFilter::AttrRarity: {
            QString converted = term.trimmed();

            /*
             * The purpose of this loop is to only apply one of the replacement
             * policies and then escape. If we attempt to layer them on top of
             * each other, we will get awkward results (i.e. comythic rare mythic rareon)
             * Conditional statement will exit once a case is successful in
             * replacement OR we go through all possible cases.
             * Will also need to replace just "mythic"
             */
            for (int i = 0; converted.length() <= 3 && i <= 6; i++) {
                switch (i) {
                    case 0:
                        converted.replace("mr", "mythic", Qt::CaseInsensitive);
                        break;
                    case 1:
                        converted.replace("m r", "mythic", Qt::CaseInsensitive);
                        break;
                    case 2:
                        converted.replace("m", "mythic", Qt::CaseInsensitive);
                        break;
                    case 3:
                        converted.replace("c", "common", Qt::CaseInsensitive);
                        break;
                    case 4:
                        converted.replace("u", "uncommon", Qt::CaseInsensitive);
                        break;
                    case 5:
                        converted.replace("r", "rare", Qt::CaseInsensitive);
                        break;
                    case 6:
                        converted.replace("s", "special", Qt::CaseInsensitive);
                        break;
                    default:
                        break;
                }
            }

            convertedRarityTerm = converted;
            break;
        }
        case CardFilter::AttrManaCost:
            sortedManaCostTerm = term.toUpper();
            // Sort the mana cost so it will be easy to find
            std::sort(sortedManaCostTerm.begin(), sortedManaCostTerm.end());
            break;
        case CardFilter::AttrLoyalty:
            uppercaseLoyaltyTerm = term.trimmed().toUpper();
            break;
        default:
            break;
    }

    // Parse the numeric relation used by cmc, power, toughness and loyalty once.
    // If the term is not numeric, it probably starts with an operator.
    const QString trimmedTerm = term.trimmed();
    bool converted = false;
    const int value = trimmedTerm.toInt(&converted);
    if (converted) {
        numericOperator = NumericOperator::Equal;
        numericOperand = value;
    } else if (trimmedTerm.size() > 1) {
        if (trimmedTerm.at(1) == '=') {
            numericOperand = trimmedTerm.mid(2).toInt();
            if (trimmedTerm.startsWith('<')) {
                numericOperator = NumericOperator::LessEqual;
            } else if (trimmedTerm.startsWith('>')) {
                numericOperator = NumericOperator::GreaterEqual;
            } else {
                numericOperator = NumericOperator::Equal;
            }
        } else {
            numericOperand = trimmedTerm.mid(1).toInt();
            if (trimmedTerm.startsWith('<')) {
                numericOperator = NumericOperator::Less;
            } else if (trimmedTerm.startsWith('>')) {
                numericOperator = NumericOperator::Greater;
            } else if (trimmedTerm.startsWith('=')) {
                numericOperator = NumericOperator::Equal;
            }
        }
    }
}

bool FilterItem::acceptName(const CardInfoPtr &info, const CardSearchLanguage &searchLanguage) const
{
    const QString &englishName = info->getName();
    const QString &localizedName = info->getLocalizedName(searchLanguage.language);

    switch (searchLanguage.mode) {
        case SearchLanguageMode::English:
            return englishName.contains(term, Qt::CaseInsensitive);
        case SearchLanguageMode::Both:
            if (englishName.contains(term, Qt::CaseInsensitive)) {
                return true;
            }
            return !searchLanguage.isEnglishOnly() && localizedName.contains(term, Qt::CaseInsensitive);
        case SearchLanguageMode::Selected:
            if (searchLanguage.isEnglishOnly()) {
                return englishName.contains(term, Qt::CaseInsensitive);
            }
            return localizedName.contains(term, Qt::CaseInsensitive);
    }

    return false;
}

bool FilterItem::acceptNameExact(const CardInfoPtr &info, const CardSearchLanguage &searchLanguage) const
{
    const QString &englishName = info->getName();
    const QString &localizedName = info->getLocalizedName(searchLanguage.language);

    switch (searchLanguage.mode) {
        case SearchLanguageMode::English:
            return englishName == term;
        case SearchLanguageMode::Both:
            if (englishName == term) {
                return true;
            }
            return !searchLanguage.isEnglishOnly() && localizedName == term;
        case SearchLanguageMode::Selected:
            if (searchLanguage.isEnglishOnly()) {
                return englishName == term;
            }
            return localizedName == term;
    }

    return false;
}

bool FilterItem::acceptType(const CardInfoPtr &info) const
{
    return info->getCardType().contains(term, Qt::CaseInsensitive);
}

bool FilterItem::acceptMainType(const CardInfoPtr &info) const
{
    const QString &type = info->getCardType();
    const qsizetype separator = type.indexOf(mainSubTypeSeparator());
    if (separator >= 0) {
        return QStringView(type).left(separator).contains(term, Qt::CaseInsensitive);
    }
    return type.contains(term, Qt::CaseInsensitive);
}

bool FilterItem::acceptSubType(const CardInfoPtr &info) const
{
    const QString &type = info->getCardType();
    const QString &separatorText = mainSubTypeSeparator();
    const qsizetype separator = type.indexOf(separatorText);
    if (separator < 0) {
        return false;
    }

    const qsizetype subTypeStart = separator + separatorText.size();
    const qsizetype nextSeparator = type.indexOf(separatorText, subTypeStart);
    const QStringView subType = nextSeparator < 0 ? QStringView(type).mid(subTypeStart)
                                                  : QStringView(type).mid(subTypeStart, nextSeparator - subTypeStart);
    return subType.contains(term, Qt::CaseInsensitive);
}

bool FilterItem::acceptColor(const CardInfoPtr &info) const
{
    const QString &colors = info->getColors();

    // Colorless card filter
    if (convertedColorTerm.toLower() == "c" && colors.length() < 1) {
        return true;
    }

    /*
     * This is a tricky part, if the filter has multiple colors in it, like UGW,
     * then we should match all of them to the card's colors
     */
    int match_count = 0;
    for (const QChar &color : convertedColorTerm) {
        if (colors.contains(color, Qt::CaseInsensitive)) {
            match_count++;
        }
    }

    return match_count == convertedColorTerm.length();
}

bool FilterItem::acceptText(const CardInfoPtr &info, const CardSearchLanguage &searchLanguage) const
{
    const QString &englishText = info->getText();
    const QString &localizedText = info->getLocalizedText(searchLanguage.language);

    switch (searchLanguage.mode) {
        case SearchLanguageMode::English:
            return englishText.contains(term, Qt::CaseInsensitive);
        case SearchLanguageMode::Both:
            if (englishText.contains(term, Qt::CaseInsensitive)) {
                return true;
            }
            return !searchLanguage.isEnglishOnly() && localizedText.contains(term, Qt::CaseInsensitive);
        case SearchLanguageMode::Selected:
            if (searchLanguage.isEnglishOnly()) {
                return englishText.contains(term, Qt::CaseInsensitive);
            }
            return localizedText.contains(term, Qt::CaseInsensitive);
    }

    return false;
}

bool FilterItem::acceptSet(const CardInfoPtr &info) const
{
    for (const auto &printings : info->getSets()) {
        for (const auto &printing : printings) {
            const CardSetPtr &set = printing.getSet();
            if (set && (set->getShortName().compare(term, Qt::CaseInsensitive) == 0 ||
                        set->getLongName().compare(term, Qt::CaseInsensitive) == 0)) {
                return true;
            }
        }
    }

    return false;
}

bool FilterItem::acceptManaCost(const CardInfoPtr &info) const
{
    // Try to seperate the mana cost in case it's a split card
    // if it's not a split card the loop will run only once
    for (const QString &manaCostPart : info->getManaCost().split("//")) {
        QString fullManaCost = manaCostPart;
        std::sort(fullManaCost.begin(), fullManaCost.end());

        // If the partial is found in the full, return true
        if (fullManaCost.contains(sortedManaCostTerm)) {
            return true;
        }
    }
    return false;
}

bool FilterItem::acceptCmc(const CardInfoPtr &info) const
{
    const QString &cmc = info->getCmc();
    bool convertSuccess;
    int cmcInt = cmc.toInt(&convertSuccess);
    // if conversion failed, check for the "//" separator used in split cards
    if (!convertSuccess) {
        int cmcSum = 0;
        for (const QString &cmcPart : cmc.split("//")) {
            cmcInt = cmcPart.toInt();
            cmcSum += cmcInt;
            if (relationCheck(cmcInt)) {
                return true;
            }
        }
        return relationCheck(cmcSum);
    } else {
        return relationCheck(cmcInt);
    }
}

bool FilterItem::acceptFormat(const CardInfoPtr &info) const
{
    return info->getLegalityProp(lowercaseTerm) == "legal";
}

bool FilterItem::acceptLoyalty(const CardInfoPtr &info) const
{
    const QString &loyaltyString = info->getLoyalty();
    if (loyaltyString.isEmpty()) {
        return false;
    }

    bool success;
    // if loyalty can't be converted to "int" it must be "X"
    int loyalty = loyaltyString.toInt(&success);
    if (success) {
        return relationCheck(loyalty);
    }
    return uppercaseLoyaltyTerm == loyaltyString;
}

bool FilterItem::acceptPowerToughness(const CardInfoPtr &info, CardFilter::Attr attr) const
{
    const QString &powTough = info->getPowTough();
    const qsizetype slash = powTough.indexOf("/");
    if (slash == -1) {
        return false;
    }

    QString valueString = attr == CardFilter::AttrPow ? powTough.mid(0, slash) : powTough.mid(slash + 1);
    if (term == valueString) {
        return true;
    }
    // advanced filtering should only happen after fast string comparison failed
    bool conversion;
    int value = valueString.toInt(&conversion);
    return conversion ? relationCheck(value) : false;
}

bool FilterItem::acceptRarity(const CardInfoPtr &info) const
{
    for (const auto &printings : info->getSets()) {
        for (const auto &printing : printings) {
            if (printing.getProperty("rarity").compare(convertedRarityTerm, Qt::CaseInsensitive) == 0) {
                return true;
            }
        }
    }
    return false;
}

bool FilterItem::relationCheck(int cardInfo) const
{
    switch (numericOperator) {
        case NumericOperator::Equal:
            return cardInfo == numericOperand;
        case NumericOperator::Less:
            return cardInfo < numericOperand;
        case NumericOperator::LessEqual:
            return cardInfo <= numericOperand;
        case NumericOperator::Greater:
            return cardInfo > numericOperand;
        case NumericOperator::GreaterEqual:
            return cardInfo >= numericOperand;
        case NumericOperator::Invalid:
            return false;
    }

    return false;
}

bool FilterItem::acceptCardAttr(const CardInfoPtr &info,
                                CardFilter::Attr attr,
                                const CardSearchLanguage &searchLanguage) const
{
    switch (attr) {
        case CardFilter::AttrName:
            return acceptName(info, searchLanguage);
        case CardFilter::AttrNameExact:
            return acceptNameExact(info, searchLanguage);
        case CardFilter::AttrType:
            return acceptType(info);
        case CardFilter::AttrColor:
            return acceptColor(info);
        case CardFilter::AttrText:
            return acceptText(info, searchLanguage);
        case CardFilter::AttrSet:
            return acceptSet(info);
        case CardFilter::AttrManaCost:
            return acceptManaCost(info);
        case CardFilter::AttrCmc:
            return acceptCmc(info);
        case CardFilter::AttrRarity:
            return acceptRarity(info);
        case CardFilter::AttrPow:
        case CardFilter::AttrTough:
            // intentional fallthrough
            return acceptPowerToughness(info, attr);
        case CardFilter::AttrLoyalty:
            return acceptLoyalty(info);
        case CardFilter::AttrFormat:
            return acceptFormat(info);
        case CardFilter::AttrMainType:
            return acceptMainType(info);
        case CardFilter::AttrSubType:
            return acceptSubType(info);
        default:
            return true; /* ignore this attribute */
    }
}

/*
 * Need to define these here to make QT happy, otherwise
 * moc doesnt find some of the FilterTreeBranch symbols.
 */
FilterTree::FilterTree() = default;
FilterTree::~FilterTree() = default;

LogicMap *FilterTree::attrLogicMap(CardFilter::Attr attr)
{
    QList<LogicMap *>::iterator i;

    int count = 0;
    for (i = childNodes.begin(); i != childNodes.end(); i++) {
        if ((*i)->attr == attr) {
            break;
        }
        count++;
    }

    if (i == childNodes.end()) {
        preInsertChild(this, count);
        i = childNodes.insert(i, new LogicMap(attr, this));
        postInsertChild(this, count);
        nodeChanged();
    }

    return *i;
}

FilterItemList *FilterTree::attrTypeList(CardFilter::Attr attr, CardFilter::Type type)
{
    return attrLogicMap(attr)->typeList(type);
}

FilterTreeNode *FilterTree::termNode(CardFilter::Attr attr, CardFilter::Type type, const QString &term)
{
    return attrTypeList(attr, type)->termNode(term);
}

FilterTreeNode *FilterTree::termNode(const CardFilter *f)
{
    return termNode(f->attr(), f->type(), f->term());
}

bool FilterTree::testAttr(const CardInfoPtr &info, const LogicMap *lm, const CardSearchLanguage &searchLanguage) const
{
    const FilterItemList *fil;
    bool status = true;

    fil = lm->findTypeList(CardFilter::TypeAnd);
    if (fil && fil->isEnabled() && !fil->testTypeAnd(info, lm->attr, searchLanguage)) {
        return false;
    }

    fil = lm->findTypeList(CardFilter::TypeAndNot);
    if (fil && fil->isEnabled() && !fil->testTypeAndNot(info, lm->attr, searchLanguage)) {
        return false;
    }

    fil = lm->findTypeList(CardFilter::TypeOr);
    if (fil && fil->isEnabled()) {
        status = false;

        // if this is true we can return because it is OR'd with the OrNot list
        if (fil->testTypeOr(info, lm->attr, searchLanguage)) {
            return true;
        }
    }

    fil = lm->findTypeList(CardFilter::TypeOrNot);
    if (fil && fil->isEnabled() && fil->testTypeOrNot(info, lm->attr, searchLanguage)) {
        return true;
    }

    return status;
}

bool FilterTree::acceptsCard(const CardInfoPtr &info, const CardSearchLanguage &searchLanguage) const
{
    for (auto i = childNodes.constBegin(); i != childNodes.constEnd(); i++) {
        if ((*i)->isEnabled() && !testAttr(info, *i, searchLanguage)) {
            return false;
        }
    }

    return true;
}

void FilterTree::removeFiltersByAttr(CardFilter::Attr filterType)
{
    for (int i = childNodes.size() - 1; i >= 0; --i) {
        auto *child = dynamic_cast<LogicMap *>(childNodes.at(i));

        if (child && child->attr == filterType) {
            deleteAt(i);
        }
    }
}

void FilterTree::removeFilter(const CardFilter *toRemove)
{
    for (int i = childNodes.size() - 1; i >= 0; --i) {
        auto *logicMap = dynamic_cast<LogicMap *>(childNodes.at(i));
        if (!logicMap || logicMap->attr != toRemove->attr()) {
            continue;
        }

        FilterItemList *typeList = logicMap->typeList(toRemove->type());
        if (!typeList) {
            continue;
        }

        int termIdx = typeList->termIndex(toRemove->term());
        if (termIdx != -1) {
            typeList->deleteAt(termIdx);
            emit typeList->nodeChanged();
            if (typeList->childCount() == 0) {
                int logicIndex = logicMap->childIndex(typeList);
                if (logicIndex != -1) {
                    logicMap->deleteAt(logicIndex);
                }
            }
        }
    }
}

void FilterTree::clear()
{
    while (childCount() > 0) {
        deleteAt(0);
    }
    emit changed();
}
