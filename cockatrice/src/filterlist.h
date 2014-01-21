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
	virtual void enable() { enabled = true; }
	virtual void disable() { enabled = false; }
	virtual FilterListNode *parent() const { return NULL; }
	virtual FilterListNode *nodeAt(int i) const { return NULL; }
	virtual void deleteAt(int i) {}
	virtual int childCount() const { return 0; }
	virtual int index() const { return -1; }
	virtual QString text() const { return ""; }
	virtual bool isLeaf() const { return false; }
	virtual const char *textCStr() const { return text().toStdString().c_str(); }
};

class FilterItemList;
class FilterList;
class LogicMap 
	: public QList<FilterItemList *>
	, public FilterListNode {
private:
	FilterList *const p;
public:
	const CardFilter::Attr attr;

	LogicMap(CardFilter::Attr a, FilterList *parent)
		: attr(a), p(parent) {}
	~LogicMap();
	const FilterItemList *findTypeList(CardFilter::Type type) const;
	FilterItemList *typeList(CardFilter::Type type);
	virtual FilterListNode *parent() const;
	virtual FilterListNode *nodeAt(int i) const;
	virtual void deleteAt(int i);
	virtual int childCount() const { return size(); }
	virtual int index() const;
	virtual QString text() const { return QString(CardFilter::attrName(attr)); }
};

class FilterItem;
class FilterItemList 
	: public QList<FilterItem *> 
	, public FilterListNode {
private:
	LogicMap *const p;
public:
	const CardFilter::Type type;

	FilterItemList(CardFilter::Type t, LogicMap *parent)
		: type(t), p(parent) {}
	~FilterItemList();
	CardFilter::Attr attr() const { return p->attr; }
	virtual FilterListNode *parent() const { return p; }
	virtual FilterListNode *nodeAt(int i) const;
	virtual void deleteAt(int i);
	virtual int childCount() const { return size(); }
	virtual int index() const { return p->indexOf((FilterItemList *) this); }
	virtual QString text() const { return QString(CardFilter::typeName(type)); }
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
	virtual FilterListNode *parent() const { return p; }
	virtual int index() const { return p->indexOf((FilterItem *)this); }
	virtual QString text() const { return term; }
	virtual bool isLeaf() const { return true; }
};

class FilterList : public QObject, public FilterListNode {
	Q_OBJECT

signals:
	void preInsertRow(const FilterListNode *parent, int i) const;
	void postInsertRow(const FilterListNode *parent, int i) const;
	void changed() const;

private:
	QList<LogicMap *> logicAttrs;

	LogicMap *attrLogicMap(CardFilter::Attr attr);
	FilterItemList *attrTypeList(CardFilter::Attr attr,
									CardFilter::Type type);

	bool acceptName(const CardInfo *info, const QString &term) const;
	bool acceptType(const CardInfo *info, const QString &term) const;
	bool acceptColor(const CardInfo *info, const QString &term) const;
	bool acceptCardAttr(const CardInfo *info, const QString &term,
						CardFilter::Attr attr) const;
	bool acceptText(const CardInfo *info, const QString &term) const;
	bool acceptSet(const CardInfo *info, const QString &term) const;
	bool acceptManaCost(const CardInfo *info, const QString &term) const;

	bool testTypeAnd(const CardInfo *info, CardFilter::Attr attr,
					const FilterItemList *fil) const;
	bool testTypeAndNot(const CardInfo *info, CardFilter::Attr attr,
							const FilterItemList *fil) const;
	bool testTypeOr(const CardInfo *info, CardFilter::Attr attr,
						const FilterItemList *filOr,
						const FilterItemList *filOrNot) const;

	bool testAttr(const CardInfo *info, const LogicMap *lm) const;
public:
	~FilterList();
	int indexOf(const LogicMap *val) const { return logicAttrs.indexOf((LogicMap *) val); }
	int findTermIndex(CardFilter::Attr attr, CardFilter::Type type,
						const QString &term);
	int findTermIndex(const CardFilter *f);
	FilterListNode *termNode(CardFilter::Attr attr, CardFilter::Type type,
						const QString &term);
	FilterListNode *termNode(const CardFilter *f);
	FilterListNode *attrTypeNode(CardFilter::Attr attr,
								CardFilter::Type type);
	int count(CardFilter::Attr attr, CardFilter::Type type);
	int count(const CardFilter *f);
	virtual FilterListNode *nodeAt(int i) const { return ((logicAttrs.size() > i)? logicAttrs.at(i) : NULL); }
	virtual void deleteAt(int i) { delete logicAttrs.takeAt(i); }
	virtual int childCount() const { return logicAttrs.size(); }
	virtual QString text() const { return QString("root"); }
	virtual int index() const { return 0; }
	void preInsertChild(const FilterListNode *p, int i) const { emit preInsertRow(p, i); }
	void postInsertChild(const FilterListNode *p, int i) const { emit postInsertRow(p, i); emit changed(); }
	void emitChanged() const { emit changed(); }

	bool acceptsCard(const CardInfo *info) const;
};

#endif
