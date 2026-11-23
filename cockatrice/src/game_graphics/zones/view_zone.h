/**
 * @file view_zone.h
 * @ingroup GameGraphicsZones
 */
//! \todo Document this file.

#ifndef ZONEVIEWERZONE_H
#define ZONEVIEWERZONE_H

#include "select_zone.h"

#include <QGraphicsItem>
#include <QGraphicsLayoutItem>
#include <QList>
#include <QLoggingCategory>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <libcockatrice/filters/filter_string.h>
#include <libcockatrice/game/board/card_list.h>
#include <qnamespace.h>
#include <qtmetamacros.h>

// IWYU pragma: keep
// ServerInfo_Card appears inside QList<...> in signal and slot
// signatures, which the moc-generated code needs to be complete.
#include <libcockatrice/protocol/pb/serverinfo_card.pb.h>

class QGraphicsItem;
class ZoneViewZoneLogic;

inline Q_LOGGING_CATEGORY(ViewZoneLog, "view_zone");

class Response;
class ServerInfo_Card;
class QGraphicsSceneWheelEvent;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

/**
 * A small chip drawn on the card at the edge of a viewed library.
 *
 * It is a sibling of the card items rather than part of the zone widget, so it
 * stays attached to its card when the view reflows into several columns or is
 * resized. ZoneViewZone owns and positions it.
 */
class LibraryEdgeMarker : public QGraphicsItem
{
public:
    explicit LibraryEdgeMarker(QGraphicsItem *parent = nullptr);

    void setLabel(const QString &label, const QString &toolTip);
    [[nodiscard]] QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QRectF markerRect;
    QString label;
};

/**
 * A CardZone that is a view into another CardZone.
 * If this zone is writable, then modifications to this zone will be reflected in the underlying zone.
 *
 * Most interactions with StackZones are handled through a zone view.
 * For example, viewing the deck/graveyard/exile is handled through a view.
 *
 * Handles both limited reveals (eg. look at top X cards) and full zone reveals (eg. searching the deck).
 */
class ZoneViewZone : public SelectZone, public QGraphicsLayoutItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsLayoutItem)
private:
    static constexpr int HORIZONTAL_PADDING = 12;
    static constexpr int VERTICAL_PADDING = 5;

    QRectF bRect, optimumRect;
    int minRows;
    FilterString filterString = FilterString("");
    CardList::SortOption groupBy, sortBy;
    bool pileView;
    LibraryEdgeMarker *libraryEdgeMarker;
    QPointF libraryEdgeMarkerAnchorPos;

    struct GridSize
    {
        int rows;
        int cols;
    };

    GridSize positionCardsForDisplay(CardList &cards, CardList::SortOption pileOption = CardList::NoSort);
    void updateLibraryEdgeMarker(const CardList &displayedCards);
    void applyLibraryEdgeMarkerText();
    void positionLibraryEdgeMarker();

    void
    handleDropEvent(const QList<CardDragItem *> &dragItems, CardZoneLogic *startZone, const QPoint &dropPoint) override;

public:
    ZoneViewZone(ZoneViewZoneLogic *_logic, QGraphicsItem *parent);
    [[nodiscard]] QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void reorganizeCards() override;
    void initializeCards(const QList<const ServerInfo_Card *> &cardList = QList<const ServerInfo_Card *>());
    void setGeometry(const QRectF &rect) override;
    [[nodiscard]] QRectF getOptimumRect() const
    {
        return optimumRect;
    }

    /** @brief Re-applies the translated library edge marker text after a language change. */
    void retranslateEdgeMarker();

public slots:
    void addToViews();
    void removeFromViews();
    void close();
    void setFilterString(const QString &_filterString);
    void setGroupBy(CardList::SortOption _groupBy);
    void setSortBy(CardList::SortOption _sortBy);
    void setPileView(int _pileView);
private slots:
    void zoneDumpReceived(const Response &r);
signals:
    void closed();
    void optimumRectChanged();
    void wheelEventReceived(QGraphicsSceneWheelEvent *event);

protected:
    [[nodiscard]] QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const override;
    void wheelEvent(QGraphicsSceneWheelEvent *event) override;
};

#endif
