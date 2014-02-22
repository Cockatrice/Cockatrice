#ifndef CARDINFOWIDGET_H
#define CARDINFOWIDGET_H

#include <QFrame>
#include <QStringList>
#include <QComboBox>

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
    qreal cardHeightOffset;
    qreal aspectRatio;
    // XXX: Why isn't this an eunm?
    int minimized; // 0 - card, 1 - oracle only, 2 - full
    ResizeMode mode;

    QComboBox *dropList;
    QLabel *cardPicture;
    QLabel *nameLabel1, *nameLabel2;
    QLabel *manacostLabel1, *manacostLabel2;
    QLabel *cardtypeLabel1, *cardtypeLabel2;
    QLabel *powtoughLabel1, *powtoughLabel2;
    QLabel *loyaltyLabel1, *loyaltyLabel2;
    QTextEdit *textLabel;

    bool shouldShowPowTough();
    bool shouldShowLoyalty();

    CardInfo *info;
    void setMinimized(int _minimized);

public:
    CardInfoWidget(ResizeMode _mode, const QString &cardName = QString(), QWidget *parent = 0, Qt::WindowFlags f = 0);
    void retranslateUi();
    QString getCardName() const;

public slots:
    void setCard(CardInfo *card);
    void setCard(const QString &cardName);
    void setCard(AbstractCardItem *card);

private slots:
    void clear();
    void updatePixmap();
    void minimizeClicked(int newMinimized);

protected:
    void resizeEvent(QResizeEvent *event);
};

#endif
