#ifndef CARDINFOWIDGET_H
#define CARDINFOWIDGET_H

#include <QFrame>

class QLabel;
class QTextEdit;
class CardItem;
class CardInfo;

class CardInfoWidget : public QFrame {
	Q_OBJECT
private:
	static const int pixmapWidth = 160;
	int pixmapHeight;

	QLabel *cardPicture;
	QLabel *nameLabel1, *nameLabel2;
	QLabel *manacostLabel1, *manacostLabel2;
	QLabel *cardtypeLabel1, *cardtypeLabel2;
	QLabel *powtoughLabel1, *powtoughLabel2;
	QTextEdit *textLabel;
	
	CardInfo *info;
public:
	CardInfoWidget(QWidget *parent = 0);
	void retranslateUi();
public slots:
	void setCard(CardInfo *card);
	void setCard(const QString &cardName);
	void setCard(CardItem *card);
private slots:
	void updatePixmap();
};

#endif
