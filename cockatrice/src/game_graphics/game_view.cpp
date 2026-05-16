#include "game_view.h"

#include "../client/settings/cache_settings.h"
#include "board/card_item.h"
#include "game_scene.h"

#include <QAction>
#include <QLabel>
#include <QMap>
#include <QResizeEvent>
#include <QRubberBand>
#include <algorithm>

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
    connect(scene, &QGraphicsScene::selectionChanged, this, [this]() { updateSelectionCount(); });

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
    const QString subtypeCountLabelStyle = baseProperties + "font-size: 12px;";

    dragCountLabel = new QLabel(this);
    dragCountLabel->setStyleSheet(dragCountLabelStyle);
    dragCountLabel->hide();
    dragCountLabel->raise();

    totalCountLabel = new QLabel(this);
    totalCountLabel->setStyleSheet(totalCountLabelStyle);
    totalCountLabel->hide();

    subtypeCountLabel = new QLabel(this);
    subtypeCountLabel->setStyleSheet(subtypeCountLabelStyle);
    subtypeCountLabel->setTextFormat(Qt::RichText);
    subtypeCountLabel->hide();
}

void GameView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);

    GameScene *s = dynamic_cast<GameScene *>(scene());
    if (s) {
        s->processViewSizeChange(event->size());
    }

    updateSceneRect(scene()->sceneRect());
    updateSelectionCount(event->size());
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

/** @brief Extracts subtypes from a card face type string (e.g., "Creature — Human Wizard" -> ["Human", "Wizard"]) */
static QStringList extractSubtypesFromFace(const QString &faceType)
{
    QStringList parts = faceType.split(QStringLiteral(" — "));
    if (parts.size() > 1) {
        return parts[1].split(QStringLiteral(" "), Qt::SkipEmptyParts);
    }
    return {};
}

QString GameView::buildSubtypeCountText() const
{
    GameScene *gameScene = dynamic_cast<GameScene *>(scene());
    if (!gameScene) {
        return QString();
    }

    // Map: main card type -> (subtype -> count)
    QMap<QString, QMap<QString, int>> subtypesByMainType;
    // Track cards contributing subtypes per main type (for group ordering)
    QMap<QString, int> cardCountPerMainType;

    for (CardItem *card : gameScene->selectedCards()) {
        if (card->getFaceDown() || card->getCard().isEmpty()) {
            continue;
        }

        QString mainType = card->getCardInfo().getMainCardType();
        if (mainType.isEmpty()) {
            mainType = QStringLiteral("Other");
        }

        QString cardType = card->getCardInfo().getCardType();
        QStringList cardFaces = cardType.split(QStringLiteral(" // "));

        bool contributedSubtypes = false;
        for (const QString &face : cardFaces) {
            QStringList subtypes = extractSubtypesFromFace(face);
            for (const QString &subtype : subtypes) {
                subtypesByMainType[mainType][subtype]++;
                contributedSubtypes = true;
            }
        }

        if (contributedSubtypes) {
            cardCountPerMainType[mainType]++;
        }
    }

    if (subtypesByMainType.isEmpty()) {
        return QString();
    }

    // Build groups with sorted subtypes
    struct MainTypeGroup
    {
        QString mainType;
        int cardCount;
        QList<QPair<QString, int>> subtypes;
    };

    QList<MainTypeGroup> groups;
    for (auto it = subtypesByMainType.constBegin(); it != subtypesByMainType.constEnd(); ++it) {
        MainTypeGroup group;
        group.mainType = it.key();
        group.cardCount = cardCountPerMainType.value(it.key(), 0);

        for (auto subIt = it.value().constBegin(); subIt != it.value().constEnd(); ++subIt) {
            group.subtypes.append({subIt.key(), subIt.value()});
        }

        /**
         * Sort subtypes: by count ascending (lower counts at top of the list), then alphabetically.
         * Since the subtype list displays above the total count label (bottom-right corner),
         * ascending order places the most common subtypes visually adjacent to the total.
         */
        std::sort(group.subtypes.begin(), group.subtypes.end(),
                  [](const QPair<QString, int> &a, const QPair<QString, int> &b) {
                      if (a.second != b.second) {
                          return a.second < b.second;
                      }
                      return a.first < b.first;
                  });

        groups.append(group);
    }

    // Sort groups: by card count ascending, then alphabetically by main type
    std::sort(groups.begin(), groups.end(), [](const MainTypeGroup &a, const MainTypeGroup &b) {
        if (a.cardCount != b.cardCount) {
            return a.cardCount < b.cardCount;
        }
        return a.mainType < b.mainType;
    });

    // Flatten to final ordered list
    QList<QPair<QString, int>> sortedEntries;
    for (const MainTypeGroup &group : groups) {
        for (const auto &entry : group.subtypes) {
            sortedEntries.append(entry);
        }
    }

    // Calculate padding widths
    int maxNameLen = 0;
    int maxCountLen = 0;
    for (const auto &entry : sortedEntries) {
        maxNameLen = qMax(maxNameLen, entry.first.length());
        maxCountLen = qMax(maxCountLen, QString::number(entry.second).length());
    }

    // Format output
    QStringList lines;
    for (const auto &entry : sortedEntries) {
        QString name = entry.first.toHtmlEscaped();
        QString count = QString::number(entry.second);

        QString namePadding = QString(QStringLiteral("&nbsp;")).repeated(maxNameLen - entry.first.length());
        QString countPadding = QString(QStringLiteral("&nbsp;")).repeated(maxCountLen - count.length());

        lines << QStringLiteral(
                     "%1%2  <span style='font-size:14px;font-weight:bold;vertical-align:middle;'>%3%4</span>")
                     .arg(namePadding, name, countPadding, count);
    }

    return lines.join(QStringLiteral("<br>"));
}

void GameView::updateSelectionCount(const QSize &viewSize)
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

    if (!SettingsCache::instance().getShowSubtypeSelectionCount() || count <= 1) {
        subtypeCountLabel->hide();
        return;
    }

    QString subtypeText = buildSubtypeCountText();

    if (subtypeText.isEmpty()) {
        subtypeCountLabel->hide();
        return;
    }

    subtypeCountLabel->setText(subtypeText);
    subtypeCountLabel->adjustSize();

    int x = availableWidth - subtypeCountLabel->width() - kMarginInPixels;
    int y;

    if (totalCountLabel->isVisible()) {
        y = totalCountLabel->y() - subtypeCountLabel->height() - kSpacingBetweenLabels;
    } else {
        y = availableHeight - subtypeCountLabel->height() - kMarginInPixels;
    }

    y = qMax(kMarginInPixels, y);

    subtypeCountLabel->move(x, y);
    subtypeCountLabel->show();
}
