/**
 * @file filter_tree.h
 * @ingroup CardDatabaseModelFilters
 */
//! \todo Document this file.

#ifndef FILTERTREE_H
#define FILTERTREE_H

#include "filter_card.h"
#include "libcockatrice/card/card_info.h"

#include <QList>
#include <QObject>
#include <QString>
#include <qtmetamacros.h>
#include <utility>

struct CardSearchLanguage;

class FilterTreeNode
{
private:
    bool enabled;

public:
    FilterTreeNode() : enabled(true)
    {
    }
    [[nodiscard]] virtual bool isEnabled() const
    {
        return enabled;
    }
    virtual void enable()
    {
        enabled = true;
        nodeChanged();
    }
    virtual void disable()
    {
        enabled = false;
        nodeChanged();
    }
    [[nodiscard]] virtual FilterTreeNode *parent() const
    {
        return nullptr;
    }
    [[nodiscard]] virtual FilterTreeNode *nodeAt(int /* i */) const
    {
        return nullptr;
    }
    virtual void deleteAt(int /* i */)
    {
    }
    [[nodiscard]] virtual int childCount() const
    {
        return 0;
    }
    virtual int childIndex(const FilterTreeNode * /* node */) const
    {
        return -1;
    }
    [[nodiscard]] virtual int index() const
    {
        return (parent() != nullptr) ? parent()->childIndex(this) : -1;
    }
    [[nodiscard]] virtual const QString text() const
    {
        return QString("");
    }
    [[nodiscard]] virtual bool isLeaf() const
    {
        return false;
    }
    virtual void nodeChanged() const
    {
        if (parent() != nullptr) {
            parent()->nodeChanged();
        }
    }
    virtual void preInsertChild(const FilterTreeNode *p, int i) const
    {
        if (parent() != nullptr) {
            parent()->preInsertChild(p, i);
        }
    }
    virtual void postInsertChild(const FilterTreeNode *p, int i) const
    {
        if (parent() != nullptr) {
            parent()->postInsertChild(p, i);
        }
    }
    virtual void preRemoveChild(const FilterTreeNode *p, int i) const
    {
        if (parent() != nullptr) {
            parent()->preRemoveChild(p, i);
        }
    }
    virtual void postRemoveChild(const FilterTreeNode *p, int i) const
    {
        if (parent() != nullptr) {
            parent()->postRemoveChild(p, i);
        }
    }
};

template <class T> class FilterTreeBranch : public FilterTreeNode
{
protected:
    QList<T> childNodes;

public:
    virtual ~FilterTreeBranch();
    void removeFiltersByAttr(CardFilter::Attr filterType);
    [[nodiscard]] FilterTreeNode *nodeAt(int i) const override;
    void deleteAt(int i) override;
    [[nodiscard]] int childCount() const override
    {
        return childNodes.size();
    }
    int childIndex(const FilterTreeNode *node) const override;
};

class FilterItemList;
class FilterTree;

class LogicMap : public FilterTreeBranch<FilterItemList *>
{

private:
    FilterTree *const p;

public:
    const CardFilter::Attr attr;

    LogicMap(CardFilter::Attr a, FilterTree *parent) : p(parent), attr(a)
    {
    }
    [[nodiscard]] const FilterItemList *findTypeList(CardFilter::Type type) const;
    FilterItemList *typeList(CardFilter::Type type);
    [[nodiscard]] FilterTreeNode *parent() const override;
    [[nodiscard]] const QString text() const override
    {
        return CardFilter::attrName(attr);
    }
};

class FilterItem;

class FilterItemList : public FilterTreeBranch<FilterItem *>
{
private:
    LogicMap *const p;

public:
    const CardFilter::Type type;

    FilterItemList(CardFilter::Type t, LogicMap *parent) : p(parent), type(t)
    {
    }
    [[nodiscard]] CardFilter::Attr attr() const
    {
        return p->attr;
    }
    [[nodiscard]] FilterTreeNode *parent() const override
    {
        return p;
    }
    [[nodiscard]] int termIndex(const QString &term) const;
    FilterTreeNode *termNode(const QString &term);
    [[nodiscard]] const QString text() const override
    {
        return CardFilter::typeName(type);
    }

    [[nodiscard]] bool
    testTypeAnd(const CardInfoPtr &info, CardFilter::Attr attr, const CardSearchLanguage &searchLanguage) const;
    [[nodiscard]] bool
    testTypeAndNot(const CardInfoPtr &info, CardFilter::Attr attr, const CardSearchLanguage &searchLanguage) const;
    [[nodiscard]] bool
    testTypeOr(const CardInfoPtr &info, CardFilter::Attr attr, const CardSearchLanguage &searchLanguage) const;
    [[nodiscard]] bool
    testTypeOrNot(const CardInfoPtr &info, CardFilter::Attr attr, const CardSearchLanguage &searchLanguage) const;
};

class FilterItem : public FilterTreeNode
{
private:
    FilterItemList *const p;

    /**
     * Values derived from the filter term that never change while this node
     * exists. Computing them in the constructor keeps them out of the per-card
     * evaluation path.
     */
    QString convertedColorTerm;
    QString convertedRarityTerm;
    QString sortedManaCostTerm;
    QString uppercaseLoyaltyTerm;
    QString lowercaseTerm;

    enum class NumericOperator
    {
        Invalid,
        Equal,
        Less,
        LessEqual,
        Greater,
        GreaterEqual,
    };
    NumericOperator numericOperator = NumericOperator::Invalid;
    int numericOperand = 0;

public:
    const QString term;

    FilterItem(QString trm, FilterItemList *parent);
    virtual ~FilterItem() = default;

    [[nodiscard]] CardFilter::Attr attr() const
    {
        return p->attr();
    }
    [[nodiscard]] CardFilter::Type type() const
    {
        return p->type;
    }
    [[nodiscard]] FilterTreeNode *parent() const override
    {
        return p;
    }
    [[nodiscard]] const QString text() const override
    {
        return term;
    }
    [[nodiscard]] bool isLeaf() const override
    {
        return true;
    }

    [[nodiscard]] bool acceptName(const CardInfoPtr &info, const CardSearchLanguage &searchLanguage) const;
    [[nodiscard]] bool acceptNameExact(const CardInfoPtr &info, const CardSearchLanguage &searchLanguage) const;
    [[nodiscard]] bool acceptType(const CardInfoPtr &info) const;
    [[nodiscard]] bool acceptMainType(const CardInfoPtr &info) const;
    [[nodiscard]] bool acceptSubType(const CardInfoPtr &info) const;
    [[nodiscard]] bool matchesColor(const QString &colors) const;
    [[nodiscard]] bool acceptColor(const CardInfoPtr &info) const;
    [[nodiscard]] bool acceptColorIdentity(const CardInfoPtr &info) const;
    [[nodiscard]] bool acceptText(const CardInfoPtr &info, const CardSearchLanguage &searchLanguage) const;
    [[nodiscard]] bool acceptSet(const CardInfoPtr &info) const;
    [[nodiscard]] bool acceptManaCost(const CardInfoPtr &info) const;
    [[nodiscard]] bool acceptCmc(const CardInfoPtr &info) const;
    [[nodiscard]] bool acceptPowerToughness(const CardInfoPtr &info, CardFilter::Attr attr) const;
    [[nodiscard]] bool acceptLoyalty(const CardInfoPtr &info) const;
    [[nodiscard]] bool acceptRarity(const CardInfoPtr &info) const;
    [[nodiscard]] bool
    acceptCardAttr(const CardInfoPtr &info, CardFilter::Attr attr, const CardSearchLanguage &searchLanguage) const;
    [[nodiscard]] bool acceptFormat(const CardInfoPtr &info) const;
    [[nodiscard]] bool acceptTag(const CardInfoPtr &info) const;
    [[nodiscard]] bool relationCheck(int cardInfo) const;
};

class FilterTree : public QObject, public FilterTreeBranch<LogicMap *>
{
    Q_OBJECT

signals:
    void preInsertRow(const FilterTreeNode *parent, int i) const;
    void postInsertRow(const FilterTreeNode *parent, int i) const;
    void preRemoveRow(const FilterTreeNode *parent, int i) const;
    void postRemoveRow(const FilterTreeNode *parent, int i) const;
    void changed() const;

private:
    LogicMap *attrLogicMap(CardFilter::Attr attr);
    FilterItemList *attrTypeList(CardFilter::Attr attr, CardFilter::Type type);

    bool testAttr(const CardInfoPtr &info, const LogicMap *lm, const CardSearchLanguage &searchLanguage) const;

    void nodeChanged() const override
    {
        emit changed();
    }
    void preInsertChild(const FilterTreeNode *p, int i) const override
    {
        emit preInsertRow(p, i);
    }
    void postInsertChild(const FilterTreeNode *p, int i) const override
    {
        emit postInsertRow(p, i);
    }
    void preRemoveChild(const FilterTreeNode *p, int i) const override
    {
        emit preRemoveRow(p, i);
    }
    void postRemoveChild(const FilterTreeNode *p, int i) const override
    {
        emit postRemoveRow(p, i);
    }

public:
    FilterTree();
    ~FilterTree() override;

    FilterTreeNode *termNode(CardFilter::Attr attr, CardFilter::Type type, const QString &term);
    FilterTreeNode *termNode(const CardFilter *f);

    [[nodiscard]] const QString text() const override
    {
        return QString("root");
    }
    [[nodiscard]] int index() const override
    {
        return 0;
    }

    [[nodiscard]] bool acceptsCard(const CardInfoPtr &info, const CardSearchLanguage &searchLanguage) const;
    void removeFiltersByAttr(CardFilter::Attr filterType);
    void removeFilter(const CardFilter *toRemove);
    void clear();
};

#endif
