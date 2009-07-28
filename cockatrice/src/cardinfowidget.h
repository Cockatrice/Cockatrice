#ifndef CARDINFOWIDGET_H
#define CARDINFOWIDGET_H

#include "carddatabase.h"
#include <QFrame>

class QLabel;
class QTextEdit;

class CardInfoWidget : public QFrame {
	Q_OBJECT
private:
	CardDatabase *db;
	static const int pixmapWidth = 180;
	int pixmapHeight;

	QLabel *cardPicture;
	QLabel *nameLabel1, *nameLabel2;
	QLabel *manacostLabel1, *manacostLabel2;
	QLabel *cardtypeLabel1, *cardtypeLabel2;
	QLabel *powtoughLabel1, *powtoughLabel2;
	QTextEdit *textLabel;
public:
	CardInfoWidget(CardDatabase *_db, QWidget *parent = 0);
	void retranslateUi();
public slots:
	void setCard(CardInfo *card);
	void setCard(const QString &cardName);
};

#endif
