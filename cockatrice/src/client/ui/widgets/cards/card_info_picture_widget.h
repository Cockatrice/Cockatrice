#ifndef CARDINFOPICTURE_H
#define CARDINFOPICTURE_H

#include "../../../../game/cards/card_database.h"
#include "card_info_picture_enlarged_widget.h"

#include <QTimer>
#include <QWidget>

class AbstractCardItem;

class CardInfoPictureWidget : public QWidget
{
    Q_OBJECT

public:
    CardInfoPictureWidget(QWidget *parent = nullptr, bool hoverToZoomEnabled = false);
    CardInfoPtr getInfo()
    {
        return info;
    }
    QSize sizeHint() const override;
    void setHoverToZoomEnabled(bool enabled);

public slots:
    void setCard(CardInfoPtr card);
    void setScaleFactor(int scale); // New slot for scaling
    void updatePixmap();

signals:
    void hoveredOnCard(CardInfoPtr hoveredCard);

protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *);
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void loadPixmap();
    const QPixmap &getResizedPixmap() const
    {
        return resizedPixmap;
    }
    void showEnlargedPixmap();

private:
    CardInfoPtr info;
    qreal magicTheGatheringCardAspectRatio = 1.396;
    qreal yuGiOhCardAspectRatio = 1.457;
    qreal aspectRatio = magicTheGatheringCardAspectRatio;
    int baseWidth = 200;
    int baseHeight = 200;
    double scaleFactor = 1.5;
    QPixmap resizedPixmap;
    bool pixmapDirty;
    bool hoverToZoomEnabled;
    int hoverActivateThresholdInMs = 500;
    CardInfoPictureEnlargedWidget *enlargedPixmapWidget;
    int enlargedPixmapOffset = 10;
    QTimer *hoverTimer;
};

#endif
