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
    QSize sizeHint() const override;
    void setHoverToZoomEnabled(bool enabled);

protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *);
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void loadPixmap();
    const QPixmap& getResizedPixmap() const { return resizedPixmap; }
    void showEnlargedPixmap();
public slots:
    void setCard(CardInfoPtr card);
    void setScaleFactor(int scale); // New slot for scaling
    void updatePixmap();

private:
    CardInfoPtr info;
    qreal aspectRatio = 1.396;
    int baseWidth = 100;
    int baseHeight = 100;
    double scaleFactor = 1.5;
    QPixmap resizedPixmap;
    bool pixmapDirty;
    bool hoverToZoomEnabled;
    CardInfoPictureEnlargedWidget *enlargedPixmapWidget;
    QTimer *hoverTimer;
};

#endif
