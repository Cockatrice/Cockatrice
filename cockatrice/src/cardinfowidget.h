#ifndef CARDINFOWIDGET_H
#define CARDINFOWIDGET_H

#include <QFrame>

class QLabel;
class QTextEdit;
class QPushButton;
class AbstractCardItem;
class CardInfo;
class QResizeEvent;
class QMouseEvent;

class CardInfoWidget : public QFrame {
	Q_OBJECT
public:
	enum ResizeMode { ModeDeckEditor, ModeGameTab, ModePopUp };
private:
	int pixmapWidth;
	qreal aspectRatio;
	bool minimized;
	ResizeMode mode;

	QPushButton *minimizeButton;
	QLabel *cardPicture;
	QLabel *nameLabel1, *nameLabel2;
	QLabel *manacostLabel1, *manacostLabel2;
	QLabel *cardtypeLabel1, *cardtypeLabel2;
	QLabel *powtoughLabel1, *powtoughLabel2;
	QTextEdit *textLabel;
	
	CardInfo *info;
	void setMinimized(bool _minimized);
public:
	CardInfoWidget(ResizeMode _mode, QWidget *parent = 0, Qt::WindowFlags f = 0);
	void retranslateUi();
public slots:
	void setCard(CardInfo *card);
	void setCard(const QString &cardName);
	void setCard(AbstractCardItem *card);
private slots:
	void updatePixmap();
	void minimizeClicked();
signals:
	void mouseReleased();
protected:
	void resizeEvent(QResizeEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
};

#endif
