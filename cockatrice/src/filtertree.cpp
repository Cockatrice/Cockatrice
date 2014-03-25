#include "filtertree.h"
#include "cardfilter.h"
#include "carddatabase.h"

#include <QList>

template <class T>
FilterTreeNode *FilterTreeBranch<T>::nodeAt(int i) const
{
    return ((childNodes.size() > i)? childNodes.at(i) : NULL);
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
    FilterTreeNode *unconst;
    T downcasted;

    /* to do the dynamic cast to T we will lose const'ness, but we can
     * trust QList::indexOf */
    unconst = (FilterTreeNode *) node;
    downcasted = dynamic_cast<T>(unconst);
    if (downcasted == NULL)
        return -1;
    return childNodes.indexOf(downcasted);
}

template <class T>
FilterTreeBranch<T>::~FilterTreeBranch()
{
    while (!childNodes.isEmpty())
        delete childNodes.takeFirst();
}

const FilterItemList *LogicMap::findTypeList(CardFilter::Type type) const
{
    QList<FilterItemList *>::const_iterator i;

    for (i = childNodes.constBegin(); i != childNodes.constEnd(); i++)
        if ((*i)->type == type)
            return *i;

    return NULL;
}

FilterItemList *LogicMap::typeList(CardFilter::Type type)
{
    QList<FilterItemList *>::iterator i;
    int count;

    count = 0;
    for (i = childNodes.begin(); i != childNodes.end(); i++) {
        if ((*i)->type == type)
            break;
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
    int i;

    for (i = 0; i < childNodes.count(); i++)
        if ((childNodes.at(i))->term == term)
            return i;

    return -1;
}

FilterTreeNode *FilterItemList::termNode(const QString &term)
{
    int i, count;
    FilterItem *fi;

    i = termIndex(term);
    if (i < 0) {
        fi = new FilterItem(term, this);
        count = childNodes.count();
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
    QList<FilterItem *>::const_iterator i;

    for (i = childNodes.constBegin(); i != childNodes.constEnd(); i++)
        if (!(*i)->acceptCardAttr(info, attr))
            return false;

    return true;
}

bool FilterItemList::testTypeAndNot(const CardInfo *info, CardFilter::Attr attr) const
{
    // if any one in the list is true, return false
    return !testTypeOr(info, attr);
}

bool FilterItemList::testTypeOr(const CardInfo *info, CardFilter::Attr attr) const
{
    QList<FilterItem *>::const_iterator i;

    for (i = childNodes.constBegin(); i != childNodes.constEnd(); i++)
        if ((*i)->acceptCardAttr(info, attr))
            return true;

    return false;
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
    QStringList::const_iterator i;
    bool status;

    status = false;
    for (i = info->getColors().constBegin(); i != info->getColors().constEnd(); i++)
        if ((*i).contains(term, Qt::CaseInsensitive)) {
            status = true;
            break;
        }

    return status;
}

bool FilterItem::acceptText(const CardInfo *info) const
{
    return info->getText().contains(term, Qt::CaseInsensitive);
}

bool FilterItem::acceptSet(const CardInfo *info) const
{
    SetList::const_iterator i;
    bool status;

    status = false;
    for (i = info->getSets().constBegin(); i != info->getSets().constEnd(); i++)
        if ((*i)->getShortName() == term
                || (*i)->getLongName().contains(term, Qt::CaseInsensitive)) {
            status = true;
            break;
        }

    return status;
}

bool FilterItem::acceptManaCost(const CardInfo *info) const
{
    return (info->getManaCost() == term);
}

bool FilterItem::acceptCardAttr(const CardInfo *info, CardFilter::Attr attr) const
{
    bool status;

    if (!isEnabled())
        return true;

    switch (attr) {
        case CardFilter::AttrName:
            status = acceptName(info);
            break;
        case CardFilter::AttrType:
            status = acceptType(info);
            break;
        case CardFilter::AttrColor:
            status = acceptColor(info);
            break;
        case CardFilter::AttrText:
            status = acceptText(info);
            break;
        case CardFilter::AttrSet:
            status = acceptSet(info);
            break;
        case CardFilter::AttrManaCost:
            status = acceptManaCost(info);
            break;
        default:
            status = true; /* ignore this attribute */
    }

    return status;
}

/* need to define these here to make QT happy, otherwise
 * moc doesnt find some of the FilterTreeBranch symbols.
 */
FilterTree::FilterTree() {}
FilterTree::~FilterTree() {}

LogicMap *FilterTree::attrLogicMap(CardFilter::Attr attr)
{
    QList<LogicMap *>::iterator i;
    int count;

    count = 0;
    for (i = childNodes.begin(); i != childNodes.end(); i++) {
        if ((*i)->attr == attr)
            break;
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

FilterItemList *FilterTree::attrTypeList(CardFilter::Attr attr,
                                            CardFilter::Type type)
{
    return attrLogicMap(attr)->typeList(type);
}

int FilterTree::findTermIndex(CardFilter::Attr attr, CardFilter::Type type,
                                const QString &term)
{
    attrTypeList(attr, type)->termIndex(term);
}

int FilterTree::findTermIndex(const CardFilter *f)
{
    return findTermIndex(f->attr(), f->type(), f->term());
}

FilterTreeNode *FilterTree::termNode(CardFilter::Attr attr, CardFilter::Type type,
                    const QString &term)
{
    return attrTypeList(attr, type)->termNode(term);
}

FilterTreeNode *FilterTree::termNode(const CardFilter *f)
{
    return termNode(f->attr(), f->type(), f->term());
}

FilterTreeNode *FilterTree::attrTypeNode(CardFilter::Attr attr,
                                CardFilter::Type type)
{
    return attrTypeList(attr, type);
}

bool FilterTree::testAttr(const CardInfo *info, const LogicMap *lm) const
{
    const FilterItemList *fil;
    bool status;

    status = true;

    fil = lm->findTypeList(CardFilter::TypeAnd);
    if (fil != NULL && fil->isEnabled() && !fil->testTypeAnd(info, lm->attr))
        return false;

    fil = lm->findTypeList(CardFilter::TypeAndNot);
    if (fil != NULL && fil->isEnabled() && !fil->testTypeAndNot(info, lm->attr))
        return false;

    fil = lm->findTypeList(CardFilter::TypeOr);
    if (fil != NULL && fil->isEnabled()) {
        status = false;
        // if this is true we can return because it is OR'd with the OrNot list
        if (fil->testTypeOr(info, lm->attr))
            return true;
    }

    fil = lm->findTypeList(CardFilter::TypeOrNot);
    if (fil != NULL && fil->isEnabled() && fil->testTypeOrNot(info, lm->attr))
        return true;

    return status;
}

bool FilterTree::acceptsCard(const CardInfo *info) const
{
    QList<LogicMap *>::const_iterator i;

    for (i = childNodes.constBegin(); i != childNodes.constEnd(); i++)
        if ((*i)->isEnabled() && !testAttr(info, *i))
            return false;

    return true;
}

void FilterTree::clear()
{
    while(childCount() > 0)
        deleteAt(0);
}
