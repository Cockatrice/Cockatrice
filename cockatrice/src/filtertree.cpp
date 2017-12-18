#include "filtertree.h"
#include "cardfilter.h"
#include "carddatabase.h"

#include <QList>

template <class T>
FilterTreeNode *FilterTreeBranch<T>::nodeAt(int i) const
{
    return (childNodes.size() > i) ? childNodes.at(i) : nullptr;
}

template <class T>
void FilterTreeBranch<T>::deleteAt(int i)
{
    preRemoveChild(this, i);
    delete childNodes.takeAt(i);
    postRemoveChild(this, i);
    nodeChanged();
}

template <class T>
int FilterTreeBranch<T>::childIndex(const FilterTreeNode *node) const
{
    FilterTreeNode *unconst = const_cast<FilterTreeNode *>(node);
    T downcasted = dynamic_cast<T>(unconst);
    return (downcasted) ? childNodes.indexOf(downcasted) : -1;
}

template <class T>
FilterTreeBranch<T>::~FilterTreeBranch()
{
    while (!childNodes.isEmpty())
    {
        delete childNodes.takeFirst();
    }
}

const FilterItemList *LogicMap::findTypeList(CardFilter::Type type) const
{
    QList<FilterItemList *>::const_iterator i;

    for (i = childNodes.constBegin(); i != childNodes.constEnd(); i++)
    {
        if ((*i)->type == type)
        {
            return *i;
        }
    }

    return nullptr;
}

FilterItemList *LogicMap::typeList(CardFilter::Type type)
{
    QList<FilterItemList *>::iterator i;
    int count = 0;

    for (i = childNodes.begin(); i != childNodes.end(); i++)
    {
        if ((*i)->type == type)
        {
            break;
        }
        count++;
    }

    if (i == childNodes.end())
    {
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
    for (int i = 0; i < childNodes.count(); i++)
    {
        if ((childNodes.at(i))->term == term)
        {
            return i;
        }
    }

    return -1;
}

FilterTreeNode *FilterItemList::termNode(const QString &term)
{
    int i = termIndex(term);
    if (i < 0)
    {
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

bool FilterItemList::testTypeAnd(const CardInfo *info, CardFilter::Attr attr) const
{
    for (auto i = childNodes.constBegin(); i != childNodes.constEnd(); i++)
    {
        if (! (*i)->isEnabled())
        {
            continue;
        }

        if (! (*i)->acceptCardAttr(info, attr))
        {
            return false;
        }
    }

    return true;
}

bool FilterItemList::testTypeAndNot(const CardInfo *info, CardFilter::Attr attr) const
{
    // if any one in the list is true, return false
    return !testTypeOr(info, attr);
}

bool FilterItemList::testTypeOr(const CardInfo *info, CardFilter::Attr attr) const
{
    bool noChildEnabledChild = true;

    for (auto i = childNodes.constBegin(); i != childNodes.constEnd(); i++)
    {
        if (! (*i)->isEnabled())
        {
            continue;
        }

        if (noChildEnabledChild)
        {
            noChildEnabledChild = false;
        }

        if ((*i)->acceptCardAttr(info, attr))
        {
            return true;
        }
    }

    return noChildEnabledChild;
}

bool FilterItemList::testTypeOrNot(const CardInfo *info, CardFilter::Attr attr) const
{
    // if any one in the list is false, return true
    return !testTypeAnd(info, attr);
}

bool FilterItem::acceptName(const CardInfo *info) const
{
    return info->getName().contains(term, Qt::CaseInsensitive);
}

bool FilterItem::acceptType(const CardInfo *info) const
{
    return info->getCardType().contains(term, Qt::CaseInsensitive);
}

bool FilterItem::acceptColor(const CardInfo *info) const
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
    if (converted_term.toLower() == "c" && info->getColors().length() < 1)
    {
        return true;
    }

    /*
     * This is a tricky part, if the filter has multiple colors in it, like UGW,
     * then we should match all of them to the card's colors
     */
    int match_count = 0;
    for (auto it = converted_term.begin(); it != converted_term.end(); it++)
    {
        for (auto i = info->getColors().constBegin(); i != info->getColors().constEnd(); i++)
        {
            if ((*i).contains((*it), Qt::CaseInsensitive))
            {
                match_count++;
            }
        }
    }

    return match_count == converted_term.length();
}

bool FilterItem::acceptText(const CardInfo *info) const
{
    return info->getText().contains(term, Qt::CaseInsensitive);
}

bool FilterItem::acceptSet(const CardInfo *info) const
{
    bool status = false;
    for (auto i = info->getSets().constBegin(); i != info->getSets().constEnd(); i++)
    {
        if ((*i)->getShortName().compare(term, Qt::CaseInsensitive) == 0
            || (*i)->getLongName().compare(term, Qt::CaseInsensitive) == 0)
        {
            status = true;
            break;
        }
    }

    return status;
}

bool FilterItem::acceptManaCost(const CardInfo *info) const
{
    return (info->getManaCost() == term);
}

bool FilterItem::acceptCmc(const CardInfo *info) const
{
    return (info->getCmc() == term);
}

bool FilterItem::acceptPower(const CardInfo *info) const
{
    int slash = info->getPowTough().indexOf("/");
    return (slash != -1) ? (info->getPowTough().mid(0,slash) == term) : false;
}

bool FilterItem::acceptToughness(const CardInfo *info) const
{
    int slash = info->getPowTough().indexOf("/");
    return (slash != -1) ? (info->getPowTough().mid(slash+1) == term) : false;
}

bool FilterItem::acceptRarity(const CardInfo *info) const
{
    QString converted_term = term.trimmed();

    /*
     * The purpose of this loop is to only apply one of the replacement
     * policies and then escape. If we attempt to layer them ontop of
     * each other, we will get awkward results (i.e. comythic rare mythic rareon)
     * Conditional statement will exit once a case is successful in
     * replacement OR we go through all possible cases.
     * Will also need to replace just "mythic"
     */
    converted_term.replace("mythic", "mythic rare", Qt::CaseInsensitive);
    for (int i = 0; converted_term.length() <= 3 && i <= 6; i++)
    {
        switch (i)
        {
            case 0: converted_term.replace("mr", "mythic rare", Qt::CaseInsensitive); break;
            case 1: converted_term.replace("m r", "mythic rare", Qt::CaseInsensitive); break;
            case 2: converted_term.replace("m", "mythic rare", Qt::CaseInsensitive); break;
            case 3: converted_term.replace("c", "common", Qt::CaseInsensitive); break;
            case 4: converted_term.replace("u", "uncommon", Qt::CaseInsensitive); break;
            case 5: converted_term.replace("r", "rare", Qt::CaseInsensitive); break;
            case 6: converted_term.replace("s", "special", Qt::CaseInsensitive); break;
            default: break;
        }
    }

    foreach (QString rareLevel, info->getRarities())
    {
        if (rareLevel.compare(converted_term, Qt::CaseInsensitive) == 0)
        {
            return true;
        }
    }
    return false;
}

bool FilterItem::acceptCardAttr(const CardInfo *info, CardFilter::Attr attr) const
{
    switch (attr)
    {
        case CardFilter::AttrName: return acceptName(info);
        case CardFilter::AttrType: return acceptType(info);
        case CardFilter::AttrColor: return acceptColor(info);
        case CardFilter::AttrText: return acceptText(info);
        case CardFilter::AttrSet: return acceptSet(info);
        case CardFilter::AttrManaCost: return acceptManaCost(info);
        case CardFilter::AttrCmc: return acceptCmc(info);
        case CardFilter::AttrRarity: return acceptRarity(info);
        case CardFilter::AttrPow: return acceptPower(info);
        case CardFilter::AttrTough: return acceptToughness(info);
        default: return true; /* ignore this attribute */
    }
}

/*
 * Need to define these here to make QT happy, otherwise
 * moc doesnt find some of the FilterTreeBranch symbols.
 */
FilterTree::FilterTree() {}
FilterTree::~FilterTree() {}

LogicMap *FilterTree::attrLogicMap(CardFilter::Attr attr)
{
    QList<LogicMap *>::iterator i;

    int count = 0;
    for (i = childNodes.begin(); i != childNodes.end(); i++)
    {
        if ((*i)->attr == attr)
        {
            break;
        }
        count++;
    }

    if (i == childNodes.end())
    {
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

bool FilterTree::testAttr(const CardInfo *info, const LogicMap *lm) const
{
    const FilterItemList *fil;
    bool status = true;

    fil = lm->findTypeList(CardFilter::TypeAnd);
    if (fil && fil->isEnabled() && !fil->testTypeAnd(info, lm->attr))
    {
        return false;
    }

    fil = lm->findTypeList(CardFilter::TypeAndNot);
    if (fil && fil->isEnabled() && !fil->testTypeAndNot(info, lm->attr))
    {
        return false;
    }

    fil = lm->findTypeList(CardFilter::TypeOr);
    if (fil && fil->isEnabled())
    {
        status = false;

        // if this is true we can return because it is OR'd with the OrNot list
        if (fil->testTypeOr(info, lm->attr))
        {
            return true;
        }
    }

    fil = lm->findTypeList(CardFilter::TypeOrNot);
    if (fil && fil->isEnabled() && fil->testTypeOrNot(info, lm->attr))
    {
        return true;
    }

    return status;
}

bool FilterTree::acceptsCard(const CardInfo *info) const
{
    for (auto i = childNodes.constBegin(); i != childNodes.constEnd(); i++)
    {
        if ((*i)->isEnabled() && !testAttr(info, *i))
        {
            return false;
        }
    }

    return true;
}

void FilterTree::clear()
{
    while (childCount() > 0)
    {
        deleteAt(0);
    }
}