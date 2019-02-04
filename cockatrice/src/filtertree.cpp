#include "filtertree.h"
#include "cardfilter.h"

#include <QList>

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

bool FilterItemList::testTypeAnd(const CardInfoPtr info, CardFilter::Attr attr) const
{
    for (auto i = childNodes.constBegin(); i != childNodes.constEnd(); i++) {
        if (!(*i)->isEnabled()) {
            continue;
        }

        if (!(*i)->acceptCardAttr(info, attr)) {
            return false;
        }
    }

    return true;
}

bool FilterItemList::testTypeAndNot(const CardInfoPtr info, CardFilter::Attr attr) const
{
    // if any one in the list is true, return false
    return !testTypeOr(info, attr);
}

bool FilterItemList::testTypeOr(const CardInfoPtr info, CardFilter::Attr attr) const
{
    bool noChildEnabledChild = true;

    for (auto i = childNodes.constBegin(); i != childNodes.constEnd(); i++) {
        if (!(*i)->isEnabled()) {
            continue;
        }

        if (noChildEnabledChild) {
            noChildEnabledChild = false;
        }

        if ((*i)->acceptCardAttr(info, attr)) {
            return true;
        }
    }

    return noChildEnabledChild;
}

bool FilterItemList::testTypeOrNot(const CardInfoPtr info, CardFilter::Attr attr) const
{
    // if any one in the list is false, return true
    return !testTypeAnd(info, attr);
}

bool FilterItem::acceptName(const CardInfoPtr info) const
{
    return info->getName().contains(term, Qt::CaseInsensitive);
}

bool FilterItem::acceptType(const CardInfoPtr info) const
{
    return info->getCardType().contains(term, Qt::CaseInsensitive);
}

bool FilterItem::acceptColor(const CardInfoPtr info) const
{
    QString converted_term = term.trimmed();

    converted_term.replace("green", "g", Qt::CaseInsensitive);
    converted_term.replace("grn", "g", Qt::CaseInsensitive);
    converted_term.replace("blue", "u", Qt::CaseInsensitive);
    converted_term.replace("blu", "u", Qt::CaseInsensitive);
    converted_term.replace("black", "b", Qt::CaseInsensitive);
    converted_term.replace("blk", "b", Qt::CaseInsensitive);
    converted_term.replace("red", "r", Qt::CaseInsensitive);
    converted_term.replace("white", "w", Qt::CaseInsensitive);
    converted_term.replace("wht", "w", Qt::CaseInsensitive);
    converted_term.replace("colorless", "c", Qt::CaseInsensitive);
    converted_term.replace("colourless", "c", Qt::CaseInsensitive);
    converted_term.replace("none", "c", Qt::CaseInsensitive);

    converted_term.replace(QString(" "), QString(""), Qt::CaseInsensitive);

    // Colorless card filter
    if (converted_term.toLower() == "c" && info->getColors().length() < 1) {
        return true;
    }

    /*
     * This is a tricky part, if the filter has multiple colors in it, like UGW,
     * then we should match all of them to the card's colors
     */
    int match_count = 0;
    for (auto &it : converted_term) {
        if (info->getColors().contains(it, Qt::CaseInsensitive))
            match_count++;
    }

    return match_count == converted_term.length();
}

bool FilterItem::acceptText(const CardInfoPtr info) const
{
    return info->getText().contains(term, Qt::CaseInsensitive);
}

bool FilterItem::acceptSet(const CardInfoPtr info) const
{
    bool status = false;
    for (const auto &set : info->getSets()) {
        if (set.getPtr()->getShortName().compare(term, Qt::CaseInsensitive) == 0 ||
            set.getPtr()->getLongName().compare(term, Qt::CaseInsensitive) == 0) {
            status = true;
            break;
        }
    }

    return status;
}

bool FilterItem::acceptManaCost(const CardInfoPtr info) const
{
    QString partialCost = term.toUpper();

    // Sort the mana cost so it will be easy to find
    std::sort(partialCost.begin(), partialCost.end());

    // Try to seperate the mana cost in case it's a split card
    // if it's not a split card the loop will run only once
    for (QString fullManaCost : info->getManaCost().split("//")) {
        std::sort(fullManaCost.begin(), fullManaCost.end());

        // If the partial is found in the full, return true
        if (fullManaCost.contains(partialCost)) {
            return true;
        }
    }
    return false;
}

bool FilterItem::acceptCmc(const CardInfoPtr info) const
{
    bool convertSuccess;
    int cmcInt = info->getCmc().toInt(&convertSuccess);
    // if conversion failed, check for the "//" separator used in split cards
    if (!convertSuccess) {
        int cmcSum = 0;
        for (const QString &cmc : info->getCmc().split("//")) {
            cmcInt = cmc.toInt();
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

bool FilterItem::acceptLoyalty(const CardInfoPtr info) const
{
    if (info->getLoyalty().isEmpty()) {
        return false;
    } else {
        bool success;
        // if loyalty can't be converted to "int" it must be "X"
        int loyalty = info->getLoyalty().toInt(&success);
        if (success) {
            return relationCheck(loyalty);
        } else {
            return term.trimmed().toUpper() == info->getLoyalty();
        }
    }
}

bool FilterItem::acceptPowerToughness(const CardInfoPtr info, CardFilter::Attr attr) const
{
    int slash = info->getPowTough().indexOf("/");
    if (slash == -1) {
        return false;
    }
    QString valueString;
    if (attr == CardFilter::AttrPow) {
        valueString = info->getPowTough().mid(0, slash);
    } else {
        valueString = info->getPowTough().mid(slash + 1);
    }
    if (term == valueString) {
        return true;
    }
    // advanced filtering should only happen after fast string comparison failed
    bool conversion;
    int value = valueString.toInt(&conversion);
    return conversion ? relationCheck(value) : false;
}

bool FilterItem::acceptRarity(const CardInfoPtr info) const
{
    QString converted_term = term.trimmed();

    /*
     * The purpose of this loop is to only apply one of the replacement
     * policies and then escape. If we attempt to layer them on top of
     * each other, we will get awkward results (i.e. comythic rare mythic rareon)
     * Conditional statement will exit once a case is successful in
     * replacement OR we go through all possible cases.
     * Will also need to replace just "mythic"
     */
    converted_term.replace("mythic", "mythic rare", Qt::CaseInsensitive);
    for (int i = 0; converted_term.length() <= 3 && i <= 6; i++) {
        switch (i) {
            case 0:
                converted_term.replace("mr", "mythic rare", Qt::CaseInsensitive);
                break;
            case 1:
                converted_term.replace("m r", "mythic rare", Qt::CaseInsensitive);
                break;
            case 2:
                converted_term.replace("m", "mythic rare", Qt::CaseInsensitive);
                break;
            case 3:
                converted_term.replace("c", "common", Qt::CaseInsensitive);
                break;
            case 4:
                converted_term.replace("u", "uncommon", Qt::CaseInsensitive);
                break;
            case 5:
                converted_term.replace("r", "rare", Qt::CaseInsensitive);
                break;
            case 6:
                converted_term.replace("s", "special", Qt::CaseInsensitive);
                break;
            default:
                break;
        }
    }

    for (const auto &set : info->getSets()) {
        if (set.getProperty("rarity").compare(converted_term, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}

bool FilterItem::relationCheck(int cardInfo) const
{
    bool result, conversion;

    // if int conversion fails, there's probably an operator at the start
    result = (cardInfo == term.toInt(&conversion));
    if (!conversion) {
        // leading whitespaces could cause indexing to fail
        QString trimmedTerm = term.trimmed();
        // check whether it's a 2 char operator (<=, >=, or ==)
        if (trimmedTerm[1] == '=') {
            int termInt = trimmedTerm.mid(2).toInt();
            if (trimmedTerm.startsWith('<')) {
                result = (cardInfo <= termInt);
            } else if (trimmedTerm.startsWith('>')) {
                result = (cardInfo >= termInt);
            } else {
                result = (cardInfo == termInt);
            }
        } else {
            int termInt = trimmedTerm.mid(1).toInt();
            if (trimmedTerm.startsWith('<')) {
                result = (cardInfo < termInt);
            } else if (trimmedTerm.startsWith('>')) {
                result = (cardInfo > termInt);
            } else if (trimmedTerm.startsWith("=")) {
                result = (cardInfo == termInt);
            } else {
                // the int conversion hasn't failed due to an operator at the start
                result = false;
            }
        }
    }
    return result;
}

bool FilterItem::acceptCardAttr(const CardInfoPtr info, CardFilter::Attr attr) const
{
    switch (attr) {
        case CardFilter::AttrName:
            return acceptName(info);
        case CardFilter::AttrType:
            return acceptType(info);
        case CardFilter::AttrColor:
            return acceptColor(info);
        case CardFilter::AttrText:
            return acceptText(info);
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

int FilterTree::findTermIndex(CardFilter::Attr attr, CardFilter::Type type, const QString &term)
{
    return attrTypeList(attr, type)->termIndex(term);
}

int FilterTree::findTermIndex(const CardFilter *f)
{
    return findTermIndex(f->attr(), f->type(), f->term());
}

FilterTreeNode *FilterTree::termNode(CardFilter::Attr attr, CardFilter::Type type, const QString &term)
{
    return attrTypeList(attr, type)->termNode(term);
}

FilterTreeNode *FilterTree::termNode(const CardFilter *f)
{
    return termNode(f->attr(), f->type(), f->term());
}

FilterTreeNode *FilterTree::attrTypeNode(CardFilter::Attr attr, CardFilter::Type type)
{
    return attrTypeList(attr, type);
}

bool FilterTree::testAttr(const CardInfoPtr info, const LogicMap *lm) const
{
    const FilterItemList *fil;
    bool status = true;

    fil = lm->findTypeList(CardFilter::TypeAnd);
    if (fil && fil->isEnabled() && !fil->testTypeAnd(info, lm->attr)) {
        return false;
    }

    fil = lm->findTypeList(CardFilter::TypeAndNot);
    if (fil && fil->isEnabled() && !fil->testTypeAndNot(info, lm->attr)) {
        return false;
    }

    fil = lm->findTypeList(CardFilter::TypeOr);
    if (fil && fil->isEnabled()) {
        status = false;

        // if this is true we can return because it is OR'd with the OrNot list
        if (fil->testTypeOr(info, lm->attr)) {
            return true;
        }
    }

    fil = lm->findTypeList(CardFilter::TypeOrNot);
    if (fil && fil->isEnabled() && fil->testTypeOrNot(info, lm->attr)) {
        return true;
    }

    return status;
}

bool FilterTree::acceptsCard(const CardInfoPtr info) const
{
    for (auto i = childNodes.constBegin(); i != childNodes.constEnd(); i++) {
        if ((*i)->isEnabled() && !testAttr(info, *i)) {
            return false;
        }
    }

    return true;
}

void FilterTree::clear()
{
    while (childCount() > 0) {
        deleteAt(0);
    }
}
