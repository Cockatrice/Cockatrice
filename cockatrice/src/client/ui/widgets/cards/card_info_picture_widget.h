#ifndef CARD_INFO_PICTURE_H
#define CARD_INFO_PICTURE_H

#include "../../../../game/cards/card_info.h"
#include "card_info_picture_enlarged_widget.h"

#include <QTimer>
#include <QWidget>

inline Q_LOGGING_CATEGORY(CardInfoPictureWidgetLog, "card_info_picture_widget");

class AbstractCardItem;
class QMenu;

class CardInfoPictureWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CardInfoPictureWidget(QWidget *parent = nullptr, bool hoverToZoomEnabled = false);
    CardInfoPtr getInfo()
    {
        return info;
    }
    [[nodiscard]] QSize sizeHint() const override;
    void setHoverToZoomEnabled(bool enabled);

public slots:
    void setCard(CardInfoPtr card);
    void setScaleFactor(int scale); // New slot for scaling
    void updatePixmap();

signals:
    void hoveredOnCard(CardInfoPtr hoveredCard);
    void cardScaleFactorChanged(int _scale);
    void cardChanged(CardInfoPtr card);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override; // Qt6 signature
#else
    void enterEvent(QEvent *event) override; // Qt5 signature
#endif
    void leaveEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void loadPixmap();
    [[nodiscard]] const QPixmap &getResizedPixmap() const
    {
        return resizedPixmap;
    }
    void showEnlargedPixmap() const;

private:
    CardInfoPtr info;
    qreal magicTheGatheringCardAspectRatio = 1.396;
    qreal yuGiOhCardAspectRatio = 1.457;
    qreal aspectRatio = magicTheGatheringCardAspectRatio;
    int baseWidth = 200;
    int baseHeight = 200;
    double scaleFactor = 100;
    QPixmap resizedPixmap;
    bool pixmapDirty;
    bool hoverToZoomEnabled;
    int hoverActivateThresholdInMs = 500;
    CardInfoPictureEnlargedWidget *enlargedPixmapWidget;
    int enlargedPixmapOffset = 10;
    QTimer *hoverTimer;

    QMenu *createRightClickMenu();
    QMenu *createViewRelatedCardsMenu();
    QMenu *createAddToOpenDeckMenu();
};

#endif
