#include "replay_event_sidebar.h"

#include "replay_event_index.h"

#include <QAbstractItemView>
#include <QFontMetrics>
#include <QFrame>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPainter>
#include <QPalette>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QVBoxLayout>
#include <algorithm>

namespace
{
enum ItemDataRole
{
    EventIndexRole = Qt::UserRole + 1,
    TimeLabelRole,
    KindRole,
    ActorRole,
    BadgeRole,
    DetailRole
};

QColor accentForKind(ReplayEventKind kind, const QPalette &palette)
{
    switch (kind) {
        case ReplayEventKind::Turn:
            return QColor(0x4c, 0xaf, 0x50);
        case ReplayEventKind::Phase:
            return QColor(0x42, 0xa5, 0xf5);
        case ReplayEventKind::Chat:
            return QColor(0xff, 0xb3, 0x00);
        case ReplayEventKind::System:
            return palette.mid().color();
        case ReplayEventKind::Action:
        default:
            return palette.highlight().color();
    }
}

class ReplayEventDelegate : public QStyledItemDelegate
{
public:
    explicit ReplayEventDelegate(QObject *parent) : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        const QPalette &palette = option.palette;
        const bool selected = option.state.testFlag(QStyle::State_Selected);
        const bool hovered = option.state.testFlag(QStyle::State_MouseOver);
        const auto kind = static_cast<ReplayEventKind>(index.data(KindRole).toInt());
        const QColor accent = accentForKind(kind, palette);

        const QRect card = option.rect.adjusted(4, 2, -4, -2);
        QColor cardBackground = selected ? palette.highlight().color() : palette.base().color();
        if (!selected && hovered) {
            cardBackground = palette.alternateBase().color();
        }

        painter->setPen(Qt::NoPen);
        painter->setBrush(cardBackground);
        painter->drawRoundedRect(card, 6, 6);

        QColor borderColor = selected ? palette.highlightedText().color() : palette.mid().color();
        borderColor.setAlpha(selected ? 100 : 130);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(borderColor, 1));
        painter->drawRoundedRect(card.adjusted(0, 0, -1, -1), 6, 6);

        QColor accentColor = accent;
        if (selected) {
            accentColor = palette.highlightedText().color();
            accentColor.setAlpha(210);
        }
        painter->setPen(Qt::NoPen);
        painter->setBrush(accentColor);
        painter->drawRoundedRect(QRect(card.left() + 7, card.top() + 7, 3, card.height() - 14), 1.5, 1.5);

        const QColor textColor = selected ? palette.highlightedText().color() : palette.text().color();
        QColor mutedColor = textColor;
        mutedColor.setAlpha(170);

        QFont timeFont = option.font;
        timeFont.setBold(true);
        timeFont.setPointSizeF(qMax(7.0, timeFont.pointSizeF() * 0.9));

        QFont actorFont = option.font;
        actorFont.setBold(true);

        QFont badgeFont = option.font;
        badgeFont.setBold(true);
        badgeFont.setPointSizeF(qMax(6.5, badgeFont.pointSizeF() * 0.72));

        QFont detailFont = option.font;
        detailFont.setPointSizeF(qMax(7.0, detailFont.pointSizeF() * 0.95));

        const QFontMetrics timeMetrics(timeFont);
        const QFontMetrics actorMetrics(actorFont);
        const QFontMetrics badgeMetrics(badgeFont);
        const QFontMetrics detailMetrics(detailFont);

        const int contentLeft = card.left() + 15;
        const int contentRight = card.right() - 10;
        const int topRowTop = card.top() + 6;
        const int topRowHeight = qMax(timeMetrics.height(), badgeMetrics.height()) + 2;

        const QString timeText = index.data(TimeLabelRole).toString();
        const QString actorText = index.data(ActorRole).toString();
        const QString badgeText = index.data(BadgeRole).toString();
        const QString detailText = index.data(DetailRole).toString();

        const int timeWidth = timeMetrics.horizontalAdvance(timeText);
        painter->setFont(timeFont);
        painter->setPen(mutedColor);
        painter->drawText(QRect(contentLeft, topRowTop, timeWidth + 2, topRowHeight), Qt::AlignLeft | Qt::AlignVCenter,
                          timeText);

        int actorRight = contentRight;
        if (!badgeText.isEmpty()) {
            const int badgeWidth = badgeMetrics.horizontalAdvance(badgeText) + 10;
            const int badgeHeight = badgeMetrics.height() + 2;
            const QRect badgeRect(contentRight - badgeWidth, topRowTop + (topRowHeight - badgeHeight) / 2, badgeWidth,
                                  badgeHeight);

            QColor badgeBackground = accentColor;
            badgeBackground.setAlpha(selected ? 45 : 32);
            painter->setPen(Qt::NoPen);
            painter->setBrush(badgeBackground);
            painter->drawRoundedRect(badgeRect, 3, 3);

            painter->setFont(badgeFont);
            painter->setPen(accentColor);
            painter->drawText(badgeRect, Qt::AlignCenter, badgeText);

            actorRight = badgeRect.left() - 8;
        }

        const int actorLeft = contentLeft + timeWidth + 10;
        if (actorRight > actorLeft) {
            const QString elidedActor =
                actorMetrics.elidedText(actorText, Qt::ElideRight, qMax(0, actorRight - actorLeft));
            painter->setFont(actorFont);
            painter->setPen(textColor);
            painter->drawText(QRect(actorLeft, topRowTop, actorRight - actorLeft, topRowHeight),
                              Qt::AlignLeft | Qt::AlignVCenter, elidedActor);
        }

        const int detailTop = topRowTop + topRowHeight + 2;
        const int detailHeight = qMax(detailMetrics.height(), card.bottom() - detailTop);
        const QString elidedDetail =
            detailMetrics.elidedText(detailText, Qt::ElideRight, qMax(0, contentRight - contentLeft));
        painter->setFont(detailFont);
        painter->setPen(textColor);
        painter->drawText(QRect(contentLeft, detailTop, contentRight - contentLeft, detailHeight),
                          Qt::AlignLeft | Qt::AlignVCenter, elidedDetail);

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex & /*index*/) const override
    {
        const QFontMetrics metrics(option.font);
        return {0, qMax(58, metrics.height() * 3 + 10)};
    }
};
} // namespace

ReplayEventSidebar::ReplayEventSidebar(QWidget *parent, const GameReplay *replay, const QList<int> &timeline)
    : QWidget(parent), eventList(new QListWidget(this)), emptyLabel(new QLabel(this))
{
    setObjectName("replayEventSidebar");

    eventList->setObjectName("replayEventList");
    eventList->setItemDelegate(new ReplayEventDelegate(eventList));
    eventList->setFrameShape(QFrame::NoFrame);
    eventList->setFocusPolicy(Qt::NoFocus);
    eventList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    eventList->setSelectionMode(QAbstractItemView::SingleSelection);
    eventList->setMouseTracking(true);
    eventList->setUniformItemSizes(true);
    eventList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLabel->setWordWrap(true);
    emptyLabel->setMargin(16);
    emptyLabel->setForegroundRole(QPalette::PlaceholderText);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(eventList);
    layout->addWidget(emptyLabel);

    const QList<ReplayEventEntry> entries = buildReplayEventEntries(replay, timeline);
    eventTimes.reserve(entries.size());

    for (const ReplayEventEntry &entry : entries) {
        auto *item = new QListWidgetItem(eventList);
        item->setData(EventIndexRole, entry.eventIndex);
        item->setData(TimeLabelRole, entry.timeLabel);
        item->setData(KindRole, static_cast<int>(entry.kind));
        item->setData(ActorRole, entry.actor);
        item->setData(BadgeRole, entry.badge);
        item->setData(DetailRole, entry.detail);
        const QString tooltip = entry.actor.isEmpty()
                                    ? tr("%1\n%2").arg(entry.timeLabel, entry.detail)
                                    : tr("%1 · %2\n%3").arg(entry.timeLabel, entry.actor, entry.detail);
        item->setToolTip(tooltip);
        item->setData(Qt::AccessibleTextRole, entry.actor.isEmpty()
                                                  ? tr("%1: %2").arg(entry.timeLabel, entry.detail)
                                                  : tr("%1, %2: %3").arg(entry.timeLabel, entry.actor, entry.detail));
        eventTimes.append(entry.timeMs);
    }

    const bool hasEvents = !entries.isEmpty();
    eventList->setVisible(hasEvents);
    emptyLabel->setVisible(!hasEvents);
    emptyLabel->setText(tr("No game events in this replay."));

    connect(eventList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        if (!item) {
            return;
        }

        setCurrentRow(eventList->row(item));
        emit eventSelected(item->data(EventIndexRole).toInt());
    });

    if (hasEvents) {
        setCurrentRow(-1);
    }
}

void ReplayEventSidebar::setCurrentTime(int timeMs)
{
    if (eventTimes.isEmpty()) {
        setCurrentRow(-1);
        return;
    }

    const auto it = std::upper_bound(eventTimes.cbegin(), eventTimes.cend(), timeMs);
    const int row = static_cast<int>(it - eventTimes.cbegin()) - 1;
    setCurrentRow(row);
}

void ReplayEventSidebar::setCurrentRow(int row)
{
    if (row < 0 || row >= eventList->count()) {
        row = -1;
    }

    if (row >= 0) {
        // The view's current index also moves on mouse press and can be deselected by a ctrl-click,
        // so the cached row alone cannot tell whether the view is already in the desired state.
        const QListWidgetItem *item = eventList->item(row);
        if (row == eventList->currentRow() && item && item->isSelected()) {
            return;
        }
    } else if (row == currentRow) {
        return;
    }

    currentRow = row;
    eventList->setCurrentRow(currentRow);

    if (currentRow >= 0) {
        QListWidgetItem *item = eventList->item(currentRow);
        if (!eventList->viewport()->rect().contains(eventList->visualItemRect(item))) {
            eventList->scrollToItem(item, QAbstractItemView::PositionAtCenter);
        }
    }
}
