#ifndef CARDINFOWIDGET_H
#define CARDINFOWIDGET_H

#include <QFrame>
#include <QStringList>
#include <QComboBox>

class CardInfo;
class CardInfoPicture;
class CardInfoText;
class AbstractCardItem;

class CardInfoWidget : public QFrame {
    Q_OBJECT

private:
    qreal aspectRatio;
    CardInfo *info;
    CardInfoPicture *pic;
    CardInfoText *text;
public:
    CardInfoWidget(const QString &cardName, QWidget *parent = 0, Qt::WindowFlags f = 0);

public slots:
    void setCard(CardInfo *card);
    void setCard(const QString &cardName);
    void setCard(AbstractCardItem *card);

private slots:
    void clear();
};

#endif
