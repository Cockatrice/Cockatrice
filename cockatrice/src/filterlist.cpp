#include "filterlist.h"
#include "CardFilter.h"

#include <QList>

LogicMap::~LogicMap()
{
	while(!isEmpty())
		delete takeFirst();
}

FilterItemList *LogicMap::typeList(CardFilter::Type type)
{
	LogicMap::iterator i;
	int count;

	count = 0;
	for(i = begin(); i != end(); i++) {
		if ((*i)->type == type)
			break;
		count++;
	}
	if (i == end()) {
		p->preInsertChild(this, count);
		i = insert(i, new FilterItemList(type, this));
		p->postInsertChild(this, count);
	}

	return *i;
}

FilterListNode *LogicMap::parent() const
{
	return p;
}

FilterListNode *LogicMap::nodeAt(int i) const
{
	return ((size() > i)? at(i) : NULL);
}

void LogicMap::deleteAt(int i)
{
	delete takeAt(i);
}

int LogicMap::index() const
{
	return p->indexOf(this);
}

FilterListNode *FilterItemList::nodeAt(int i) const
{
	return ((size() > i)? at(i) : NULL);
}

void FilterItemList::deleteAt(int i) {
	delete takeAt(i);
}

FilterItemList::~FilterItemList()
{
	while(!isEmpty())
		delete takeFirst();
}

FilterList::~FilterList()
{
	while(!logicAttrs.isEmpty()) {
		delete logicAttrs.takeFirst();
	}
}

LogicMap *FilterList::attrLogicMap(CardFilter::Attr attr)
{
	QList<LogicMap *>::iterator i;
	int count;

	count = 0;
	for(i = logicAttrs.begin(); i != logicAttrs.end(); i++) {
		if((*i)->attr == attr)
			break;
		count++;
	}

	if(i == logicAttrs.end()) {
		preInsertChild(this, count);
		i = logicAttrs.insert(i, new LogicMap(attr, this));
		postInsertChild(this, count);
	}

	return *i;
}

FilterItemList *FilterList::attrTypeList(CardFilter::Attr attr,
											CardFilter::Type type)
{
	return attrLogicMap(attr)->typeList(type);
}

int FilterList::findTermIndex(CardFilter::Attr attr, CardFilter::Type type,
								const QString &term)
{
	FilterItemList *fis;
	int i;

	fis = attrTypeList(attr, type);
	for(i = 0; i < fis->count(); i++)
		if((fis->at(i))->term == term)
			return i;

	return -1;
}

int FilterList::findTermIndex(const CardFilter *f)
{
	return findTermIndex(f->attr(), f->type(), f->term());
}

FilterListNode *FilterList::termNode(CardFilter::Attr attr, CardFilter::Type type,
					const QString &term)
{
	FilterItemList *fis;
	FilterItem *fi;
	int i, count;

	fis = attrTypeList(attr, type);
	i = findTermIndex(attr, type, term);
	if(i < 0) {
		fi = new FilterItem(term, fis);
		count = fis->childCount();
		preInsertChild(fis, count);
		fis->append(fi);
		postInsertChild(fis, count);
		return fi;
	}

	return fis->at(i);
}

FilterListNode *FilterList::termNode(const CardFilter *f)
{
	return termNode(f->attr(), f->type(), f->term());
}

FilterListNode *FilterList::attrTypeNode(CardFilter::Attr attr,
								CardFilter::Type type)
{
	return attrTypeList(attr, type);
}

int FilterList::count(CardFilter::Attr attr, CardFilter::Type type)
{
	return attrTypeList(attr, type)->count();
}

int FilterList::count(const CardFilter *f)
{
	return count(f->attr(), f->type());
}
