#ifndef FILTERLIST_H
#define FILTERLIST_H

#include <QList>
#include <QMap>
#include <QObject>

#include "cardfilter.h"

class CardInfo;

class FilterListNode {
private:
	bool enabled;
public:
	FilterListNode() : enabled(true) {}
	virtual bool isEnabled() const { return enabled; }
	virtual void enable() { enabled = true; nodeChanged(); }
	virtual void disable() { enabled = false; nodeChanged(); }
	virtual FilterListNode *parent() const { return NULL; }
	virtual FilterListNode *nodeAt(int i) const { return NULL; }
	virtual void deleteAt(int i) {}
	virtual int childCount() const { return 0; }
	virtual int childIndex(const FilterListNode *node) const { return -1; }
	virtual int index() const { return (parent() != NULL)? parent()->childIndex(this) : -1; }
	virtual QString text() const { return ""; }
	virtual bool isLeaf() const { return false; }
	virtual const char *textCStr() const { return text().toStdString().c_str(); }
	virtual void nodeChanged() const { 
		printf("%s -> ", textCStr());
		if (parent() != NULL) parent()->nodeChanged();
	}
	virtual	void preInsertChild(const FilterListNode *p, int i) const {
		//printf("%s -> ", textCStr());
		if (parent() != NULL) parent()->preInsertChild(p, i);
	}
	virtual	void postInsertChild(const FilterListNode *p, int i) const {
		//printf("%s -> ", textCStr());
		if (parent() != NULL) parent()->postInsertChild(p, i);
	}
	virtual	void preRemoveChild(const FilterListNode *p, int i) const {
		printf("%s -> ", textCStr());
		if (parent() != NULL) parent()->preRemoveChild(p, i);
	}
	virtual	void postRemoveChild(const FilterListNode *p, int i) const {
		printf("%s -> ", textCStr());
		if (parent() != NULL) parent()->postRemoveChild(p, i);
	}
};

template <class T>
class FilterListInnerNode : public FilterListNode {
protected:
	QList<T> childNodes;
public:
	~FilterListInnerNode();
	FilterListNode *nodeAt(int i) const;
	void deleteAt(int i);
	int childCount() const { return childNodes.size(); }
	int childIndex(const FilterListNode *node) const;
};

class FilterItemList;
class FilterList;
class LogicMap : public FilterListInnerNode<FilterItemList *> {

private:
	FilterList *const p;

public:
	const CardFilter::Attr attr;

	LogicMap(CardFilter::Attr a, FilterList *parent)
		: attr(a), p(parent) {}
	const FilterItemList *findTypeList(CardFilter::Type type) const;
	FilterItemList *typeList(CardFilter::Type type);
	FilterListNode *parent() const;
	QString text() const { return QString(CardFilter::attrName(attr)); }
};

class FilterItem;
class FilterItemList : public FilterListInnerNode<FilterItem *> {
private:
	LogicMap *const p;
public:
	const CardFilter::Type type;

	FilterItemList(CardFilter::Type t, LogicMap *parent)
		: type(t), p(parent) {}
	CardFilter::Attr attr() const { return p->attr; }
	FilterListNode *parent() const { return p; }
	int termIndex(const QString &term) const;
	FilterListNode *termNode(const QString &term);
	QString text() const { return QString(CardFilter::typeName(type)); }

	bool testTypeAnd(const CardInfo *info, CardFilter::Attr attr) const;
	bool testTypeAndNot(const CardInfo *info, CardFilter::Attr attr) const;
	bool testTypeOr(const CardInfo *info, CardFilter::Attr attr) const;
	bool testTypeOrNot(const CardInfo *info, CardFilter::Attr attr) const;
};

class FilterItem : public FilterListNode {
private:
	FilterItemList *const p;
public:
	const QString term;

	FilterItem(QString trm, FilterItemList *parent)
		: p(parent), term(trm) {}

	CardFilter::Attr attr() const { return p->attr(); }
	CardFilter::Type type() const { return p->type; }
	FilterListNode *parent() const { return p; }
	QString text() const { return term; }
	bool isLeaf() const { return true; }

	bool acceptName(const CardInfo *info) const;
	bool acceptType(const CardInfo *info) const;
	bool acceptColor(const CardInfo *info) const;
	bool acceptText(const CardInfo *info) const;
	bool acceptSet(const CardInfo *info) const;
	bool acceptManaCost(const CardInfo *info) const;
	bool acceptCardAttr(const CardInfo *info, CardFilter::Attr attr) const;
};

class FilterList : public QObject, public FilterListInnerNode<LogicMap *> {
	Q_OBJECT

signals:
	void preInsertRow(const FilterListNode *parent, int i) const;
	void postInsertRow(const FilterListNode *parent, int i) const;
	void preRemoveRow(const FilterListNode *parent, int i) const;
	void postRemoveRow(const FilterListNode *parent, int i) const;
	void changed() const;

private:
	LogicMap *attrLogicMap(CardFilter::Attr attr);
	FilterItemList *attrTypeList(CardFilter::Attr attr,
									CardFilter::Type type);

	bool testAttr(const CardInfo *info, const LogicMap *lm) const;
public:
	FilterList();
	~FilterList();
	int findTermIndex(CardFilter::Attr attr, CardFilter::Type type,
						const QString &term);
	int findTermIndex(const CardFilter *f);
	FilterListNode *termNode(CardFilter::Attr attr, CardFilter::Type type,
						const QString &term);
	FilterListNode *termNode(const CardFilter *f);
	FilterListNode *attrTypeNode(CardFilter::Attr attr,
								CardFilter::Type type);
	QString text() const { return QString("root"); }
	int index() const { return 0; }

	void nodeChanged() const { printf("root\n"); emit changed(); }
	void preInsertChild(const FilterListNode *p, int i) const { emit preInsertRow(p, i); }
	void postInsertChild(const FilterListNode *p, int i) const { emit postInsertRow(p, i); }
	void preRemoveChild(const FilterListNode *p, int i) const { printf("root\n"); emit preRemoveRow(p, i); }
	void postRemoveChild(const FilterListNode *p, int i) const { printf("root\n"); emit postRemoveRow(p, i); }

	bool acceptsCard(const CardInfo *info) const;
};

#endif
