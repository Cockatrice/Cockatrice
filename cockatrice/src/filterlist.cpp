#include "filterlist.h"
#include "cardfilter.h"
#include "carddatabase.h"

#include <QList>

LogicMap::~LogicMap()
{
	while(!isEmpty())
		delete takeFirst();
}

const FilterItemList *LogicMap::findTypeList(CardFilter::Type type) const
{
	LogicMap::const_iterator i;

	for(i = constBegin(); i != constEnd(); i++)
		if ((*i)->type == type)
			return *i;

	return NULL;
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

bool FilterList::acceptName(const CardInfo *info, const QString &term) const
{
	return info->getName().contains(term, Qt::CaseInsensitive);
}

bool FilterList::acceptType(const CardInfo *info, const QString &term) const
{
	return info->getCardType().contains(term, Qt::CaseInsensitive);
}

bool FilterList::acceptColor(const CardInfo *info, const QString &term) const
{
	QStringList::const_iterator i;
	bool status;

	status = false;
	for(i = info->getColors().constBegin(); i != info->getColors().constEnd(); i++)
		if ((*i).contains(term, Qt::CaseInsensitive)) {
			status = true;
			break;
		}

	return status;
}

bool FilterList::acceptText(const CardInfo *info, const QString &term) const
{
	return info->getText().contains(term, Qt::CaseInsensitive);
}

bool FilterList::acceptSet(const CardInfo *info, const QString &term) const
{
	SetList::const_iterator i;
	bool status;

	status = false;
	for(i = info->getSets().constBegin(); i != info->getSets().constEnd(); i++)
		if ((*i)->getShortName() == term
				|| (*i)->getLongName().contains(term, Qt::CaseInsensitive)) {
			status = true;
			break;
		}

	return status;
}

bool FilterList::acceptManaCost(const CardInfo *info, const QString &term) const
{
	return (info->getManaCost() == term);
}

bool FilterList::acceptCardAttr(const CardInfo *info, const QString &term,
								CardFilter::Attr attr) const
{
	bool status;

	switch(attr) {
		case CardFilter::AttrName:
			status = acceptName(info, term);
			break;
		case CardFilter::AttrType:
			status = acceptType(info, term);
			break;
		case CardFilter::AttrColor:
			status = acceptColor(info, term);
			break;
		case CardFilter::AttrText:
			status = acceptText(info, term);
			break;
		case CardFilter::AttrSet:
			status = acceptSet(info, term);
			break;
		case CardFilter::AttrManaCost:
			status = acceptManaCost(info, term);
			break;
		default:
			status = true; /* ignore this attribute */
	}

	return status;
}

bool FilterList::testTypeAnd(const CardInfo *info, CardFilter::Attr attr,
							const FilterItemList *fil) const
{
	FilterItemList::const_iterator i;

	for(i = fil->constBegin(); i != fil->constEnd(); i++)
		if(!acceptCardAttr(info, (*i)->term, attr))
			return false;

	return true;
}

bool FilterList::testTypeAndNot(const CardInfo *info, CardFilter::Attr attr,
									const FilterItemList *fil) const
{
	FilterItemList::const_iterator i;

	for(i = fil->constBegin(); i != fil->constEnd(); i++)
		if(acceptCardAttr(info, (*i)->term, attr))
			return false;

	return true;
}

bool FilterList::testTypeOr(const CardInfo *info, CardFilter::Attr attr,
								const FilterItemList *filOr,
								const FilterItemList *filOrNot) const
{
	FilterItemList::const_iterator i;
	bool status;

	if(filOr == NULL && filOrNot == NULL)
		return true;

	status = false;
	if (filOr != NULL)
		for(i = filOr->constBegin(); i != filOr->constEnd(); i++)
			if(acceptCardAttr(info, (*i)->term, attr)) {
				status = true;
				break;
			}
	if (status != true && filOrNot != NULL)
		for(i = filOrNot->constBegin(); i != filOrNot->constEnd(); i++)
			if(!acceptCardAttr(info, (*i)->term, attr)) {
				status = true;
				break;
			}

	return status;
}

bool FilterList::testAttr(const CardInfo *info, const LogicMap *lm) const
{
	bool status;
	const FilterItemList *fil, *fil2;

	fil = lm->findTypeList(CardFilter::TypeAnd);
	if (fil != NULL && !testTypeAnd(info, lm->attr, fil))
		return false;

	fil = lm->findTypeList(CardFilter::TypeAndNot);
	if (fil != NULL && !testTypeAndNot(info, lm->attr, fil))
		return false;

	fil = lm->findTypeList(CardFilter::TypeOr);
	fil2 = lm->findTypeList(CardFilter::TypeOrNot);
	if (!testTypeOr(info, lm->attr, fil, fil2))
		return false;

	return true;
}

bool FilterList::acceptsCard(const CardInfo *info) const
{
	QList<LogicMap *>::const_iterator i;

	for(i = logicAttrs.constBegin(); i != logicAttrs.constEnd(); i++)
		if(!testAttr(info, *i))
			return false;

	return true;
}
