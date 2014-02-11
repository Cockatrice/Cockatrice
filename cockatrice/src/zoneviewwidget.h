#ifndef ZONEVIEWWIDGET_H
#define ZONEVIEWWIDGET_H

#include <QGraphicsWidget>

class QLabel;
class QPushButton;
class CardZone;
class ZoneViewZone;
class Player;
class CardDatabase;
class QScrollBar;
class QCheckBox;
class GameScene;
class ServerInfo_Card;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneWheelEvent;

class TitleLabel : public QGraphicsWidget {
    Q_OBJECT
private:
    QString text;
    QPointF buttonDownPos;
public:
    TitleLabel();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/);
    void setText(const QString &_text) { text = _text; update(); }
signals:
    void mouseMoved(QPointF scenePos);
protected:
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
};

class ZoneViewWidget : public QGraphicsWidget {
    Q_OBJECT
private:
    ZoneViewZone *zone;
    QGraphicsWidget *zoneContainer;
    
    TitleLabel *titleLabel;
    QPushButton *closeButton;
    QScrollBar *scrollBar;
    QCheckBox *sortByNameCheckBox, *sortByTypeCheckBox, *shuffleCheckBox;
    
    int extraHeight;
    Player *player;
signals:
    void closePressed(ZoneViewWidget *zv);
private slots:
    void resizeToZoneContents();
    void handleWheelEvent(QGraphicsSceneWheelEvent *event);
    void handleScrollBarChange(int value);
    void zoneDeleted();
    void moveWidget(QPointF scenePos);
public:
    ZoneViewWidget(Player *_player, CardZone *_origZone, int numberCards = 0, bool _revealZone = false, bool _writeableRevealZone = false, const QList<const ServerInfo_Card *> &cardList = QList<const ServerInfo_Card *>());
    ZoneViewZone *getZone() const { return zone; }
    void retranslateUi();
protected:
    void closeEvent(QCloseEvent *event);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif
