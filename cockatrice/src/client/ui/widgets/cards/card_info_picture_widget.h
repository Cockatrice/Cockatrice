#ifndef CARDINFOPICTURE_H
#define CARDINFOPICTURE_H

#include "../../../../game/cards/card_database.h"
#include "card_info_picture_enlarged_widget.h"

#include <QWidget>
#include <QTimer>

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
    const QPixmap& getResizedPixmap() const { return resizedPixmap; }
    void showEnlargedPixmap();


private:
    CardInfoPtr info;
    qreal aspectRatio = 1.396;
    int baseWidth = 200;
    int baseHeight = 200;
    double scaleFactor = 1.5;
    QPixmap resizedPixmap;
    bool pixmapDirty;
    bool hoverToZoomEnabled;
    CardInfoPictureEnlargedWidget *enlargedPixmapWidget;
    QTimer *hoverTimer;
};

#endif
