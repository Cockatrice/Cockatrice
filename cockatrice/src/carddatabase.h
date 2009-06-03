#ifndef CARDDATABASE_H
#define CARDDATABASE_H

#include <QHash>
#include <QPixmap>
#include <QMap>
#include <QDataStream>

class CardInfo {
private:
	QString name;
	QStringList editions;
	QString manacost;
	QString cardtype;
	QString powtough;
	QStringList text;
	QPixmap *pixmap;
	QMap<int, QPixmap *> scaledPixmapCache;
public:
	CardInfo(const QString &_name = QString(),
		const QString &_manacost = QString(),
		const QString &_cardtype = QString(),
		const QString &_powtough = QString(),
		const QStringList &_text = QStringList());
	CardInfo(QDataStream &stream);
	~CardInfo();
	QString getName() const { return name; }
	QStringList getEditions() const { return editions; }
	QString getManacost() const { return manacost; }
	QString getCardType() const { return cardtype; }
	QString getPowTough() const { return powtough; }
	QStringList getText() const { return text; }
	QString getMainCardType() const;
	void addEdition(const QString &edition);
	QPixmap *loadPixmap();
	QPixmap *getPixmap(QSize size);
	void saveToStream(QDataStream &stream);
};

class CardDatabase {
private:
	QHash<QString, CardInfo *> hash;
	static const unsigned int magicNumber = 0x12345678;
	static const unsigned int fileVersion = 1;
public:
	CardDatabase();
	~CardDatabase();
	void clear();
	CardInfo *getCard(const QString &cardName = QString());
	QList<CardInfo *> getCardList();
	void importOracle();
	int loadFromFile(const QString &fileName);
	bool saveToFile(const QString &fileName);
};

#endif
