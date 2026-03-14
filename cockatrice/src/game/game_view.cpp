#include "game_view.h"

#include "../client/settings/cache_settings.h"
#include "game_scene.h"

#include <QAction>
#include <QLabel>
#include <QResizeEvent>
#include <QRubberBand>

// QRubberBand calls raise() in showEvent() and changeEvent() to stay on top of siblings.
// This subclass disables that behavior so dragCountLabel can appear above it.
class SelectionRubberBand : public QRubberBand
{
public:
    using QRubberBand::QRubberBand;

protected:
    void showEvent(QShowEvent *event) override
    {
        QWidget::showEvent(event); // Skip QRubberBand's raise()
    }

    void changeEvent(QEvent *event) override
    {
        if (event->type() == QEvent::ZOrderChange) {
            return; // Skip QRubberBand's raise() on z-order changes
        }
        QRubberBand::changeEvent(event);
    }
};

GameView::GameView(GameScene *scene, QWidget *parent) : QGraphicsView(scene, parent), rubberBand(0)
{
    setBackgroundBrush(QBrush(QColor(0, 0, 0)));
    setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
    setFocusPolicy(Qt::ClickFocus);
    setViewportUpdateMode(BoundingRectViewportUpdate);

    connect(scene, &GameScene::sceneRectChanged, this, &GameView::updateSceneRect);

    connect(scene, &GameScene::sigStartRubberBand, this, &GameView::startRubberBand);
    connect(scene, &GameScene::sigResizeRubberBand, this, &GameView::resizeRubberBand);
    connect(scene, &GameScene::sigStopRubberBand, this, &GameView::stopRubberBand);
    connect(scene, &QGraphicsScene::selectionChanged, this, [this]() { updateTotalSelectionCount(); });

    aCloseMostRecentZoneView = new QAction(this);

    connect(aCloseMostRecentZoneView, &QAction::triggered, scene, &GameScene::closeMostRecentZoneView);
    addAction(aCloseMostRecentZoneView);
    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &GameView::refreshShortcuts);
    refreshShortcuts();
    rubberBand = new SelectionRubberBand(QRubberBand::Rectangle, this);

    const QString countLabelStyle = "color: white; "
                                    "font-size: 14px; "
                                    "font-weight: bold; "
                                    "background-color: rgba(0, 0, 0, 160); "
                                    "border-radius: 3px; "
                                    "padding: 1px 2px;";

    dragCountLabel = new QLabel(this);
    dragCountLabel->setStyleSheet(countLabelStyle);
    dragCountLabel->hide();
    dragCountLabel->raise();

    totalCountLabel = new QLabel(this);
    totalCountLabel->setStyleSheet(countLabelStyle);
    totalCountLabel->hide();
}

void GameView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);

    GameScene *s = dynamic_cast<GameScene *>(scene());
    if (s)
        s->processViewSizeChange(event->size());

    updateSceneRect(scene()->sceneRect());
    updateTotalSelectionCount(event->size());
}

void GameView::updateSceneRect(const QRectF &rect)
{
    fitInView(rect, Qt::KeepAspectRatio);
}

void GameView::startRubberBand(const QPointF &_selectionOrigin)
{
    if (!rubberBand)
        return;

    selectionOrigin = _selectionOrigin;
    rubberBand->setGeometry(QRect(mapFromScene(selectionOrigin), QSize(0, 0)));
    rubberBand->show();
}

void GameView::resizeRubberBand(const QPointF &cursorPoint, int selectedCount)
{
    if (!rubberBand)
        return;

    constexpr int kLabelPaddingInPixels = 4;

    QPoint cursor = cursorPoint.toPoint();
    QRect rect = QRect(mapFromScene(selectionOrigin), cursor).normalized();
    rubberBand->setGeometry(rect);

    if (!SettingsCache::instance().getShowDragSelectionCount()) {
        dragCountLabel->hide();
        return;
    }

    if (selectedCount > 0) {
        dragCountLabel->setText(QString::number(selectedCount));
        dragCountLabel->adjustSize();
        QSize labelSize = dragCountLabel->size();

        if (rect.width() < labelSize.width() + 2 * kLabelPaddingInPixels ||
            rect.height() < labelSize.height() + 2 * kLabelPaddingInPixels) {
            dragCountLabel->hide();
            return;
        }

        const int minX = rect.left() + kLabelPaddingInPixels;
        const int minY = rect.top() + kLabelPaddingInPixels;

        int x = qMax(minX, cursor.x() - labelSize.width() - kLabelPaddingInPixels);
        int y = qMax(minY, cursor.y() - labelSize.height() - kLabelPaddingInPixels);

        bool isAtTopLeftCorner = (x == minX) && (y == minY);
        if (isAtTopLeftCorner) {
            constexpr int kCursorClearanceInPixels = 16;
            x = qMin(cursor.x() + kCursorClearanceInPixels, rect.right() - labelSize.width() - kLabelPaddingInPixels);
        }

        dragCountLabel->move(x, y);
        dragCountLabel->show();
    } else {
        dragCountLabel->hide();
    }
}

void GameView::stopRubberBand()
{
    if (!rubberBand)
        return;

    rubberBand->hide();
    dragCountLabel->hide();
}

void GameView::refreshShortcuts()
{
    aCloseMostRecentZoneView->setShortcuts(
        SettingsCache::instance().shortcuts().getShortcut("Player/aCloseMostRecentZoneView"));
}

void GameView::updateTotalSelectionCount(const QSize &viewSize)
{
    if (!SettingsCache::instance().getShowTotalSelectionCount()) {
        totalCountLabel->hide();
        return;
    }

    int count = scene()->selectedItems().count();

    if (count > 1) {
        totalCountLabel->setText(QString::number(count));
        totalCountLabel->adjustSize();

        constexpr int kMarginInPixels = 10;
        int availableWidth = viewSize.isValid() ? viewSize.width() : viewport()->width();
        int availableHeight = viewSize.isValid() ? viewSize.height() : viewport()->height();
        int x = availableWidth - totalCountLabel->width() - kMarginInPixels;
        int y = availableHeight - totalCountLabel->height() - kMarginInPixels;
        totalCountLabel->move(x, y);
        totalCountLabel->show();
    } else {
        totalCountLabel->hide();
    }
}
