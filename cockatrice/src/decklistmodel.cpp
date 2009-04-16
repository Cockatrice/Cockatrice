#include <QFile>
#include <QTextStream>
#include "decklistmodel.h"
#include "carddatabase.h"

DeckListModel::DeckListModel(CardDatabase *_db, QObject *parent)
	: QAbstractListModel(parent), db(_db)
{

}

DeckListModel::~DeckListModel()
{
	qDebug("DeckListModel destructor");
	cleanList();
}

int DeckListModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return deckList.size();
}

int DeckListModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return 2;
}

QVariant DeckListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();
	if ((index.row() >= deckList.size()) || (index.column() >= 2))
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();

	DecklistRow *r = deckList.at(index.row());
	switch (index.column()) {
		case 0: return r->getNumber();
		case 1: return r->getCard();
		default: return QVariant();
	}
}

QVariant DeckListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();
	if (orientation != Qt::Horizontal)
		return QVariant();
	switch (section) {
		case 0: return QString(tr("Number"));
		case 1: return QString(tr("Card"));
		default: return QVariant();
	}
}

void DeckListModel::cleanList()
{
	QListIterator<DecklistRow *> i(deckList);
	while (i.hasNext())
		delete i.next();
	deckList.clear();
	reset();
}

bool DeckListModel::loadFromFile(const QString &fileName)
{
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	QTextStream in(&file);
	cleanList();
	while (!in.atEnd()) {
		QString line = in.readLine().simplified();
		bool isSideboard = false;
		if (line.startsWith("SB:", Qt::CaseInsensitive)) {
			line = line.mid(3).trimmed();
			isSideboard = true;
		}
		int i = line.indexOf(' ');
		bool ok;
		int number = line.left(i).toInt(&ok);
		if (!ok)
			continue;
		DecklistRow *row = new DecklistRow(number, line.mid(i + 1), isSideboard);
		deckList << row;
	}
	cacheCardPictures();
	reset();
	return true;
}

bool DeckListModel::saveToFile(const QString &fileName)
{
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return false;
	QTextStream out(&file);
	QListIterator<DecklistRow *> i(deckList);
	while (i.hasNext()) {
		DecklistRow *r = i.next();
		out << QString("%1%2 %3\n").arg(r->isSideboard() ? "SB: " : "").arg(r->getNumber()).arg(r->getCard());
	}
	return true;
}

DecklistRow *DeckListModel::getRow(int row) const
{
	if (row >= deckList.size())
		return 0;
	return deckList.at(row);
}

void DeckListModel::cacheCardPictures()
{
	QListIterator<DecklistRow *> i(deckList);
	while (i.hasNext()) {
		DecklistRow *r = i.next();
		db->getCard(r->getCard())->getPixmap();
	}
}
