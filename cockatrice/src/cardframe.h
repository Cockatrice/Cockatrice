#ifndef CARDFRAME_H
#define CARDFRAME_H

#include <QLabel>
#include <QStringList>

class AbstractCardItem;
class CardInfo;
class QResizeEvent;

class CardFrame : public QLabel {
	Q_OBJECT

private:
	CardInfo *info;

public:
	CardFrame(const QString &cardName = QString(), QWidget *parent = 0);
	QString getCardName() const;

public slots:
	void setCard(CardInfo *card);
	void setCard(const QString &cardName);
	void setCard(AbstractCardItem *card);

private slots:
	void clear();
	void updatePixmap();

};

#endif
