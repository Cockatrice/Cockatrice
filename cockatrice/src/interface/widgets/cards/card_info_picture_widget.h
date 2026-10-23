/**
 * @file card_info_picture_widget.h
 * @ingroup CardWidgets
 */
//! \todo Document this file.

#ifndef CARD_INFO_PICTURE_H
#define CARD_INFO_PICTURE_H

#include <QLoggingCategory>
#include <QMouseEvent>
#include <QPixmap>
#include <QPoint>
#include <QWidget>
#include <libcockatrice/card/printing/exact_card.h>
#include <qtmetamacros.h>

class CardInfoPictureEnlargedWidget;
class QPropertyAnimation;
class QTimer;

inline Q_LOGGING_CATEGORY(CardInfoPictureWidgetLog, "card_info_picture_widget");

class QMenu;

class CardInfoPictureWidget : public QWidget
{
    Q_OBJECT

public:
    /// Width of a card picture at 100 % scale, in pixels.
    static constexpr int BASE_WIDTH = 200;
    /// Height / width of a card picture.
    static constexpr qreal ASPECT_RATIO = 1.396;

    explicit CardInfoPictureWidget(QWidget *parent = nullptr,
                                   bool hoverToZoomEnabled = false,
                                   bool raiseOnEnter = false);
    ExactCard getCard()
    {
        return exactCard;
    }
    [[nodiscard]] QSize sizeHint() const override;

    /**
     * @brief The size a card picture has at @p scaleFactor percent.
     * @param scaleFactor Percentage of BASE_WIDTH, as set by the card size slider.
     */
    [[nodiscard]] static QSize sizeForScaleFactor(int scaleFactor);

public slots:
    void setCard(const ExactCard &card);
    void setScaleFactor(int scale); // New slot for scaling
    void setHoverToZoomEnabled(bool enabled);
    void setRaiseOnEnterEnabled(bool enabled);
    void updatePixmap();

signals:
    void hoveredOnCard(const ExactCard &hoveredCard);
    void cardScaleFactorChanged(int _scale);
    void cardChanged(const ExactCard &card);
    void cardClicked(QMouseEvent *event, const ExactCard &card);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void loadPixmap();
    [[nodiscard]] const QPixmap &getResizedPixmap() const
    {
        return resizedPixmap;
    }
    void showEnlargedPixmap();
    void destroyEnlargedPixmapWidget();

private:
    ExactCard exactCard;
    double scaleFactor = 100;
    QPixmap resizedPixmap;
    bool pixmapDirty;
    bool hoverToZoomEnabled;
    bool raiseOnEnter;

    CardInfoPictureEnlargedWidget *enlargedPixmapWidget = nullptr;

    QTimer *hoverTimer;
    QPropertyAnimation *animation;
    QPoint originalPos; // Store the original position

    QMenu *createRightClickMenu();
    QMenu *createViewRelatedCardsMenu();
    QMenu *createAddToOpenDeckMenu();
};

#endif
