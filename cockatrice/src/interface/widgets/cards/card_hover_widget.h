/**
 * @file card_hover_widget.h
 * @ingroup CardWidgets
 * @brief Hover popup: zoomed picture plus a vertical icon nav strip beside stacked action pages.
 *
 * The board card smooth-zooms first; this panel then swaps in over it with oracle
 * text and every right-click action, grouped by topic.
 */

#ifndef CARDHOVERWIDGET_H
#define CARDHOVERWIDGET_H

#include <QCursor>
#include <QFrame>
#include <QPointer>
#include <QRect>
#include <QSize>
#include <QTransform>
#include <functional>
#include <libcockatrice/card/printing/exact_card.h>

class CardInfoPictureWidget;
class CardInfoTextWidget;
class CardItem;
class CardState;
class PlayerGraphicsItem;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QToolButton;
class QButtonGroup;
class QStackedWidget;
class QLabel;
class QLineEdit;
class QEnterEvent;
class QHideEvent;
class QPropertyAnimation;
class QResizeEvent;
class QTimer;

class CardHoverWidget : public QFrame
{
    Q_OBJECT

public:
    explicit CardHoverWidget(QWidget *parent = nullptr);

    void setCard(CardItem *card, PlayerGraphicsItem *player);
    void clearCard();
    bool hasCard() const;
    CardItem *getCardItem() const
    {
        return currentCard;
    }

    void positionNextTo(const QRectF &cardSceneRect, const QTransform &viewportTransform, const QWidget *viewport);

    void showAnimated();
    void hideAnimated();

    /** @brief True while the panel is fading out towards a hide. */
    bool isClosing() const;

    /** @brief Refreshes the PT, counter, and annotation labels from the current card. */
    void refreshDisplay();

    bool isMouseOverWidget() const
    {
        return rect().contains(mapFromGlobal(QCursor::pos()));
    }

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void changeEvent(QEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void hideEvent(QHideEvent *event) override;

signals:
    void hoverEntered();
    void hoverLeft();

private slots:
    void onCardChanged();
    void onCardDestroyed();
    void applyThemeStyle();

private:
    QPointer<CardItem> currentCard;
    QPointer<PlayerGraphicsItem> currentPlayer;
    ExactCard shownCard;

    // Which side of the card the panel ended up on (drives the image-adjacent layout flip)
    bool popupOnRight = true;

    // Viewport area the panel covered at its last placement; used to repaint what the
    // overlay window uncovers when it moves or hides.
    QRect lastViewportRect;

    // Coalesces rapid state-change bursts into a single refresh
    QTimer *refreshTimer;

    // Card image area (kept on the card-adjacent side) with overlaid controls
    QHBoxLayout *mainLayout;
    QWidget *imageContainer;
    CardInfoPictureWidget *pic;

    // Name bar overlay (top of card image)
    QWidget *nameBarOverlay;
    QLabel *nameLabel;
    QLineEdit *nameEdit;
    QLabel *annotationLabel;

    // P/T overlay (bottom-right of card image): a compact value pill at rest that
    // lifts its control rows out of itself while the P/T area is hovered.
    QWidget *ptOverlay;
    QWidget *ptControls;
    QLabel *ptPillLabel;
    bool ptExpanded = false;
    QSize ptCompactSize;
    QSize ptFullSize;
    QLabel *ptPowerLabel;
    QLabel *ptToughLabel;
    QPushButton *btnIncP;
    QPushButton *btnDecP;
    QPushButton *btnIncT;
    QPushButton *btnDecT;
    QPushButton *btnSetPT;
    QPushButton *btnResetPT;

    // Vertical icon nav strip plus stacked content pages (no nested tab bars)
    QWidget *sideBar;
    QLabel *navNameLabel;
    QToolButton *navInfoBtn;
    QToolButton *navMainBtn;
    QToolButton *navMoveBtn;
    QToolButton *navCounterBtn;
    QToolButton *navTokenBtn;
    QButtonGroup *navGroup;

    QStackedWidget *contentStack;
    QWidget *infoPage;
    QWidget *mainPage;
    QWidget *movePage;
    QWidget *counterPage;
    QWidget *tokenPage;
    QVBoxLayout *infoPageLayout;
    QVBoxLayout *mainPageLayout;
    QVBoxLayout *movePageLayout;
    QVBoxLayout *counterPageLayout;
    QVBoxLayout *tokenPageLayout;
    CardInfoTextWidget *text;

    // Window fade (sole reveal animation: per-layer opacity effects flashed black
    // on scroll areas, so the whole panel fades as one)
    QPropertyAnimation *fadeIn;
    QPropertyAnimation *fadeOut;

    void createImageArea();
    void createNameBarOverlay();
    void createPTOverlay();
    void createNavBar();
    QToolButton *createNavButton(const QString &title, const QString &iconName, int pageIndex);
    void createContentStack();
    QWidget *makeStackPage(QVBoxLayout *&layout);
    void showPage(int index);

    QLabel *createSectionHeader(const QString &text, QWidget *parent);
    QLabel *createNoteLabel(const QString &text, QWidget *parent);

    void connectCardSignals();
    void disconnectCardSignals();
    void scheduleRefresh();

    void rebuildActionsForZone();
    void buildMainTab(const QList<CardState *> &cardList, const QString &zoneName, bool canModify);
    void buildMoveTab(const QList<CardState *> &cardList, const QString &zoneName, bool canModify);
    void buildCounterTab(const QList<CardState *> &cardList, const QString &zoneName, bool canModify);
    void buildTokenTab(CardItem *card, bool canModify);
    void addRelatedViewButtons(QVBoxLayout *layout);
    void updatePTDisplay();
    void positionOverlays();
    /** @brief Shows/hides the P/T control rows; the value pill stays at the bottom. */
    void setPTExpanded(bool expanded);
    /** @brief Anchors the pill on the printed P/T box, or the expanded stack above it. */
    void applyPTOverlayGeometry();
    /** @brief Resting pill geometry, in @c imageContainer coordinates. */
    QRect compactPillRect() const;
    /** @brief Pill plus halo: the only area that opens the rows, in @c pic coordinates. */
    QRect ptOpenRegion() const;
    /** @brief Bottom-right band the rows stay open in (never opens them), in @c pic coordinates. */
    QRect ptHoverRegion() const;
    /** @brief True while the cursor is over the overlay or the printed P/T area. */
    bool isCursorOverPTArea() const;
    /** @brief Re-measures the resting pill from its current text. */
    QSize measureCompactPTSize() const;
    /** @brief Repaints the viewport area this window covers and the one it just left. */
    void invalidateCoveredArea();

    void clearLayout(QLayout *layout);

    QPushButton *createOverlayButton(const QString &text, const QString &tooltip, const std::function<void()> &slot);
    QToolButton *createGridButton(const QString &text,
                                  const QString &iconName,
                                  const std::function<void()> &slot,
                                  QWidget *parent,
                                  const QString &tooltip = QString());
    QPushButton *createSmallButton(const QString &text, const std::function<void()> &slot, QWidget *parent = nullptr);
    QPushButton *createSmallButton(const QString &text,
                                   const QString &tooltip,
                                   const std::function<void()> &slot,
                                   QWidget *parent = nullptr);
};

#endif // CARDHOVERWIDGET_H
