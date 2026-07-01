#include "game_view.h"

#include "../client/settings/cache_settings.h"
#include "game_scene.h"

#include <QAction>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QResizeEvent>
#include <QRubberBand>
#include <libcockatrice/utility/qt_utils.h>

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
    setViewportUpdateMode(BoundingRectViewportUpdate);

    connect(scene, &GameScene::sceneRectChanged, this, &GameView::updateSceneRect);

    connect(scene, &GameScene::sigStartRubberBand, this, &GameView::startRubberBand);
    connect(scene, &GameScene::sigResizeRubberBand, this, &GameView::resizeRubberBand);
    connect(scene, &GameScene::sigStopRubberBand, this, &GameView::stopRubberBand);
    connect(scene, &QGraphicsScene::selectionChanged, this, [this]() { updateTotalSelectionCount(); });

    setFocusDisabled(SettingsCache::instance().getKeepGameChatFocus());
    connect(&SettingsCache::instance(), &SettingsCache::keepGameChatFocusChanged, this, &GameView::setFocusDisabled);

    aCloseMostRecentZoneView = new QAction(this);

    connect(aCloseMostRecentZoneView, &QAction::triggered, scene, &GameScene::closeMostRecentZoneView);
    addAction(aCloseMostRecentZoneView);
    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &GameView::refreshShortcuts);
    refreshShortcuts();
    rubberBand = new SelectionRubberBand(QRubberBand::Rectangle, this);

    const QString baseProperties = "color: white; "
                                   "font-family: monospace; "
                                   "background-color: rgba(0, 0, 0, 160); "
                                   "border-radius: 3px; "
                                   "padding: 1px 2px; "
                                   "white-space: pre;";

    const QString dragCountLabelStyle = baseProperties + "font-size: 14px; font-weight: bold;";
    const QString totalCountLabelStyle = baseProperties + "font-size: 16px; font-weight: bold;";
    const QString subtypeTallyLabelStyle = baseProperties + "font-size: 12px;";

    dragCountLabel = new QLabel(this);
    dragCountLabel->setStyleSheet(dragCountLabelStyle);
    dragCountLabel->hide();
    dragCountLabel->raise();

    totalCountLabel = new QLabel(this);
    totalCountLabel->setStyleSheet(totalCountLabelStyle);
    totalCountLabel->hide();

    tallyContainer = new QWidget(this);
    tallyContainer->setStyleSheet(subtypeTallyLabelStyle);
    tallyLayout = new QGridLayout(tallyContainer);
    tallyLayout->setContentsMargins(2, 2, 2, 2);
    tallyLayout->setSpacing(2);
    tallyContainer->hide();
}

void GameView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);

    GameScene *s = static_cast<GameScene *>(scene());
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
    if (!rubberBand) {
        return;
    }

    selectionOrigin = _selectionOrigin;
    rubberBand->setGeometry(QRect(mapFromScene(selectionOrigin), QSize(0, 0)));
    rubberBand->show();
}

void GameView::resizeRubberBand(const QPointF &cursorPoint, int selectedCount)
{
    if (!rubberBand) {
        return;
    }

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
    if (!rubberBand) {
        return;
    }

    rubberBand->hide();
    dragCountLabel->hide();
}

void GameView::refreshShortcuts()
{
    aCloseMostRecentZoneView->setShortcuts(
        SettingsCache::instance().shortcuts().getShortcut("Player/aCloseMostRecentZoneView"));
}

void GameView::clearTallyLabels()
{
    QtUtils::clearLayoutRec(tallyLayout);
}

QSize GameView::rebuildTallyLabels(const QList<TallyRow> &entries)
{
    clearTallyLabels();

    const QString nameStyle = QStringLiteral("color: white; font-size: 12px; background: transparent;");
    const QString countStyle =
        QStringLiteral("color: white; font-size: 14px; font-weight: bold; background: transparent;");

    int totalHeight = 0;
    int maxNameWidth = 0;
    int maxCountWidth = 0;

    int row = 0;
    for (const TallyRow &entry : entries) {
        auto *nameLabel = new QLabel(entry.name, tallyContainer);
        nameLabel->setStyleSheet(nameStyle);
        nameLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        tallyLayout->addWidget(nameLabel, row, 0);

        auto *countLabel = new QLabel(entry.value, tallyContainer);
        countLabel->setStyleSheet(countStyle);
        countLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        tallyLayout->addWidget(countLabel, row, 1);

        QSize nameSize = nameLabel->sizeHint();
        QSize countSize = countLabel->sizeHint();
        maxNameWidth = qMax(maxNameWidth, nameSize.width());
        maxCountWidth = qMax(maxCountWidth, countSize.width());
        totalHeight += qMax(nameSize.height(), countSize.height());

        ++row;
    }

    int spacing = tallyLayout->spacing();
    int margins = tallyLayout->contentsMargins().left() + tallyLayout->contentsMargins().right();
    int verticalMargins = tallyLayout->contentsMargins().top() + tallyLayout->contentsMargins().bottom();

    int width = maxNameWidth + spacing + maxCountWidth + margins;
    int height = totalHeight + (row - 1) * spacing + verticalMargins;

    return QSize(width, height);
}

void GameView::updateTotalSelectionCount(const QSize &viewSize)
{
    constexpr int kMarginInPixels = 10;
    constexpr int kSpacingBetweenLabels = 4;

    int availableWidth = viewSize.isValid() ? viewSize.width() : viewport()->width();
    int availableHeight = viewSize.isValid() ? viewSize.height() : viewport()->height();

    int count = scene()->selectedItems().count();

    if (!SettingsCache::instance().getShowTotalSelectionCount() || count <= 1) {
        totalCountLabel->hide();
    } else {
        totalCountLabel->setText(QString::number(count));
        totalCountLabel->adjustSize();

        int x = availableWidth - totalCountLabel->width() - kMarginInPixels;
        int y = availableHeight - totalCountLabel->height() - kMarginInPixels;
        totalCountLabel->move(x, y);
        totalCountLabel->show();
    }

    TallyType tallyType =
        SettingsCache::instance().getShowSubtypeSelectionTally() ? TallyType::Subtypes : TallyType::None;

    GameScene *gameScene = static_cast<GameScene *>(scene());
    QList<TallyRow> entries = Tally::compute(gameScene->selectedCards(), tallyType);

    if (entries.isEmpty() || count <= 1) {
        tallyContainer->hide();
        cachedTallyRows.clear();
        return;
    }

    // Only rebuild labels if entries changed
    QSize containerSize;
    if (entries != cachedTallyRows) {
        cachedTallyRows = entries;
        containerSize = rebuildTallyLabels(entries);
        tallyContainer->resize(containerSize);
    } else {
        containerSize = tallyContainer->size();
    }

    int x = availableWidth - containerSize.width() - kMarginInPixels;
    int y;

    if (totalCountLabel->isVisible()) {
        y = totalCountLabel->y() - containerSize.height() - kSpacingBetweenLabels;
    } else {
        y = availableHeight - containerSize.height() - kMarginInPixels;
    }

    y = qMax(kMarginInPixels, y);

    tallyContainer->move(x, y);
    tallyContainer->show();
}

/**
 * Disabling focus on the game view will allow chat to maintain the autofocusing behavior of pre 2.10.3,
 * at the cost of disabling the zone view search bar.
 */
void GameView::setFocusDisabled(bool disabled)
{
    setFocusPolicy(disabled ? Qt::NoFocus : Qt::ClickFocus);
}
