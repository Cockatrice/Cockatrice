#ifndef CARDINFOWIDGET_H
#define CARDINFOWIDGET_H

#include <QFrame>

class QLabel;
class QTextEdit;
class QPushButton;
class AbstractCardItem;
class CardInfo;

class CardInfoWidget : public QFrame {
	Q_OBJECT
private:
	static const int pixmapWidth = 160;
	int pixmapHeight;
	bool minimized;

	QPushButton *minimizeButton;
	QLabel *cardPicture;
	QLabel *nameLabel1, *nameLabel2;
	QLabel *manacostLabel1, *manacostLabel2;
	QLabel *cardtypeLabel1, *cardtypeLabel2;
	QLabel *powtoughLabel1, *powtoughLabel2;
	QTextEdit *textLabel;
	
	CardInfo *info;
public:
	CardInfoWidget(bool showMinimizeButton = true, QWidget *parent = 0, Qt::WindowFlags f = 0);
	void retranslateUi();
public slots:
	void setCard(CardInfo *card);
	void setCard(const QString &cardName);
	void setCard(AbstractCardItem *card);
private slots:
	void updatePixmap();
	void minimizeClicked();
};

#endif
