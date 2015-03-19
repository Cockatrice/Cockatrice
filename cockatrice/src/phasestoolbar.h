#ifndef PHASESTOOLBAR_H
#define PHASESTOOLBAR_H

#include <QFrame>
#include <QList>
#include <QGraphicsObject>

namespace google { namespace protobuf { class Message; } }
class Player;
class GameCommand;

class PhaseButton : public QObject, public QGraphicsItem {
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
private:
    QString name;
    bool active, highlightable;
    int activeAnimationCounter;
    QTimer *activeAnimationTimer;
    QAction *doubleClickAction;
    double width;
    
    void updatePixmap(QPixmap &pixmap);
private slots:
    void updateAnimation();
public:
    PhaseButton(const QString &_name, QGraphicsItem *parent = 0, QAction *_doubleClickAction = 0, bool _highlightable = true);
    QRectF boundingRect() const;
    void setWidth(double _width);
    void setActive(bool _active);
    bool getActive() const { return active; }
    void triggerDoubleClickAction();
signals:
    void clicked();
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
};

class PhasesToolbar : public QObject, public QGraphicsItem {
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
private:
    QList<PhaseButton *> buttonList;
    PhaseButton *nextTurnButton;
    double width, height, ySpacing, symbolSize;
    static const int buttonCount = 12;
    static const int spaceCount = 6;
    static const double marginSize;
    void rearrangeButtons();
public:
    PhasesToolbar(QGraphicsItem *parent = 0);
    QRectF boundingRect() const;
    void retranslateUi();
    void setHeight(double _height);
    double getWidth() const { return width; }
    int phaseCount() const { return buttonList.size(); }
    QString getLongPhaseName(int phase) const;
public slots:
    void setActivePhase(int phase);
private slots:
    void phaseButtonClicked();
    void actNextTurn();
    void actUntapAll();
    void actDrawCard();
signals:
    void sendGameCommand(const ::google::protobuf::Message &command, int playerId);
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/);
};

#endif
