#ifndef FILTERTREE_H
#define FILTERTREE_H

#include <QList>
#include <QMap>
#include <QObject>

#include "cardfilter.h"

class CardInfo;

class FilterTreeNode {
private:
    bool enabled;
public:
    FilterTreeNode() : enabled(true) {}
    virtual bool isEnabled() const { return enabled; }
    virtual void enable() { enabled = true; nodeChanged(); }
    virtual void disable() { enabled = false; nodeChanged(); }
    virtual FilterTreeNode *parent() const { return NULL; }
    virtual FilterTreeNode *nodeAt(int /* i */) const { return NULL; }
    virtual void deleteAt(int /* i */) {}
    virtual int childCount() const { return 0; }
    virtual int childIndex(const FilterTreeNode * /* node */) const { return -1; }
    virtual int index() const { return (parent() != NULL)? parent()->childIndex(this) : -1; }
    virtual QString text() const { return QString(textCStr()); }
    virtual bool isLeaf() const { return false; }
    virtual const char *textCStr() const { return ""; }
    virtual void nodeChanged() const {
        if (parent() != NULL) parent()->nodeChanged();
    }
    virtual    void preInsertChild(const FilterTreeNode *p, int i) const {
        if (parent() != NULL) parent()->preInsertChild(p, i);
    }
    virtual    void postInsertChild(const FilterTreeNode *p, int i) const {
        if (parent() != NULL) parent()->postInsertChild(p, i);
    }
    virtual    void preRemoveChild(const FilterTreeNode *p, int i) const {
        if (parent() != NULL) parent()->preRemoveChild(p, i);
    }
    virtual    void postRemoveChild(const FilterTreeNode *p, int i) const {
        if (parent() != NULL) parent()->postRemoveChild(p, i);
    }
};

template <class T>
class FilterTreeBranch : public FilterTreeNode {
protected:
    QList<T> childNodes;
public:
    virtual ~FilterTreeBranch();
    FilterTreeNode *nodeAt(int i) const;
    void deleteAt(int i);
    int childCount() const { return childNodes.size(); }
    int childIndex(const FilterTreeNode *node) const;
};

class FilterItemList;
class FilterTree;
class LogicMap : public FilterTreeBranch<FilterItemList *> {

private:
    FilterTree *const p;

public:
    const CardFilter::Attr attr;

    LogicMap(CardFilter::Attr a, FilterTree *parent)
        : p(parent), attr(a) {}
    const FilterItemList *findTypeList(CardFilter::Type type) const;
    FilterItemList *typeList(CardFilter::Type type);
    FilterTreeNode *parent() const;
    const char* textCStr() const { return CardFilter::attrName(attr); }
};

class FilterItem;
class FilterItemList : public FilterTreeBranch<FilterItem *> {
private:
    LogicMap *const p;
public:
    const CardFilter::Type type;

    FilterItemList(CardFilter::Type t, LogicMap *parent)
        : p(parent), type(t) {}
    CardFilter::Attr attr() const { return p->attr; }
    FilterTreeNode *parent() const { return p; }
    int termIndex(const QString &term) const;
    FilterTreeNode *termNode(const QString &term);
    const char *textCStr() const { return CardFilter::typeName(type); }

    bool testTypeAnd(const CardInfo *info, CardFilter::Attr attr) const;
    bool testTypeAndNot(const CardInfo *info, CardFilter::Attr attr) const;
    bool testTypeOr(const CardInfo *info, CardFilter::Attr attr) const;
    bool testTypeOrNot(const CardInfo *info, CardFilter::Attr attr) const;
};

class FilterItem : public FilterTreeNode {
private:
    FilterItemList *const p;
public:
    const QString term;

    FilterItem(QString trm, FilterItemList *parent)
        : p(parent), term(trm) {}
    virtual ~FilterItem() {};

    CardFilter::Attr attr() const { return p->attr(); }
    CardFilter::Type type() const { return p->type; }
    FilterTreeNode *parent() const { return p; }
    QString text() const { return term; }
    const char *textCStr() const { return term.toStdString().c_str(); }
    bool isLeaf() const { return true; }

    bool acceptName(const CardInfo *info) const;
    bool acceptType(const CardInfo *info) const;
    bool acceptColor(const CardInfo *info) const;
    bool acceptText(const CardInfo *info) const;
    bool acceptSet(const CardInfo *info) const;
    bool acceptManaCost(const CardInfo *info) const;
    bool acceptCmc(const CardInfo *info) const;
    bool acceptRarity(const CardInfo *info) const;
    bool acceptCardAttr(const CardInfo *info, CardFilter::Attr attr) const;
};

class FilterTree : public QObject, public FilterTreeBranch<LogicMap *> {
    Q_OBJECT

signals:
    void preInsertRow(const FilterTreeNode *parent, int i) const;
    void postInsertRow(const FilterTreeNode *parent, int i) const;
    void preRemoveRow(const FilterTreeNode *parent, int i) const;
    void postRemoveRow(const FilterTreeNode *parent, int i) const;
    void changed() const;

private:
    LogicMap *attrLogicMap(CardFilter::Attr attr);
    FilterItemList *attrTypeList(CardFilter::Attr attr,
                                    CardFilter::Type type);

    bool testAttr(const CardInfo *info, const LogicMap *lm) const;

    void nodeChanged() const { emit changed(); }
    void preInsertChild(const FilterTreeNode *p, int i) const { emit preInsertRow(p, i); }
    void postInsertChild(const FilterTreeNode *p, int i) const { emit postInsertRow(p, i); }
    void preRemoveChild(const FilterTreeNode *p, int i) const { emit preRemoveRow(p, i); }
    void postRemoveChild(const FilterTreeNode *p, int i) const { emit postRemoveRow(p, i); }

public:
    FilterTree();
    ~FilterTree();
    int findTermIndex(CardFilter::Attr attr, CardFilter::Type type,
                        const QString &term);
    int findTermIndex(const CardFilter *f);
    FilterTreeNode *termNode(CardFilter::Attr attr, CardFilter::Type type,
                        const QString &term);
    FilterTreeNode *termNode(const CardFilter *f);
    FilterTreeNode *attrTypeNode(CardFilter::Attr attr,
                                CardFilter::Type type);
    const char *textCStr() const { return "root"; }
    int index() const { return 0; }

    bool acceptsCard(const CardInfo *info) const;
    void clear();
};

#endif
