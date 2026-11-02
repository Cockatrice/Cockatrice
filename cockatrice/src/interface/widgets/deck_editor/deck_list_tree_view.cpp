#include "deck_list_tree_view.h"

#include <QAbstractItemDelegate>
#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QStyleOptionViewItem>
#include <libcockatrice/models/deck_list/deck_list_model.h>

DeckListTreeView::DeckListTreeView(QWidget *parent) : QTreeView(parent)
{
    const auto repaint = [this] { viewport()->update(); };
    connect(header(), &QHeaderView::sectionResized, this, repaint);
    connect(header(), &QHeaderView::sectionMoved, this, repaint);
    connect(header(), &QHeaderView::geometriesChanged, this, repaint);
    connect(this, &QTreeView::expanded, this, repaint);
    connect(this, &QTreeView::collapsed, this, repaint);
}

void DeckListTreeView::setModel(QAbstractItemModel *newModel)
{
    for (const auto &connection : modelConnections) {
        disconnect(connection);
    }
    modelConnections.clear();
    pressedHeader = QModelIndex();
    headerPress = false;
    branchPress = false;
    QTreeView::setModel(newModel);
    if (newModel) {
        const auto repaint = [this] { viewport()->update(); };
        modelConnections.append(connect(newModel, &QAbstractItemModel::dataChanged, this, repaint));
        modelConnections.append(connect(newModel, &QAbstractItemModel::modelReset, this, repaint));
        modelConnections.append(connect(newModel, &QAbstractItemModel::layoutChanged, this, repaint));
        modelConnections.append(connect(newModel, &QAbstractItemModel::rowsInserted, this, repaint));
        modelConnections.append(connect(newModel, &QAbstractItemModel::rowsRemoved, this, repaint));
        modelConnections.append(connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, repaint));
        modelConnections.append(connect(selectionModel(), &QItemSelectionModel::currentChanged, this, repaint));
    }
    viewport()->update();
}

DeckListTreeView::StickyHeader DeckListTreeView::stickyHeader() const
{
    if (!model() || viewport()->height() == 0) {
        return {};
    }
    QModelIndex top;
    // A visible section is needed even when columns are moved, hidden, or scrolled horizontally.
    for (int column = 0; column < header()->count(); ++column) {
        if (isColumnHidden(column)) {
            continue;
        }
        const int x = qMax(0, header()->sectionViewportPosition(column));
        if (x < viewport()->width() && header()->sectionViewportPosition(column) + columnWidth(column) > 0) {
            top = QTreeView::indexAt(QPoint(x, 0));
            break;
        }
    }
    if (!top.isValid()) {
        return {};
    }
    const int geometryColumn = top.column();
    top = top.siblingAtColumn(0);
    const QModelIndex category = top.data(DeckRoles::IsCardRole).toBool() ? top.parent() : top;
    const QRect original = visualRect(category.siblingAtColumn(geometryColumn));
    if (!category.isValid() || original.top() >= 0 || original.height() <= 0) {
        return {};
    }
    const int height = original.height();
    int y = 0;
    for (auto next = indexBelow(top); next.isValid(); next = indexBelow(next)) {
        const int nextY = visualRect(next.siblingAtColumn(geometryColumn)).top();
        if (nextY >= height) {
            break;
        }
        if (!next.data(DeckRoles::IsCardRole).toBool()) {
            y = qMin(0, nextY - height);
            break;
        }
    }
    return {category, QRect(0, y, viewport()->width(), height)};
}

int DeckListTreeView::treeColumn() const
{
    return treePosition() < 0 ? header()->logicalIndex(0) : treePosition();
}

int DeckListTreeView::headerIndent(const QModelIndex &index) const
{
    int levels = rootIsDecorated() ? 1 : 0;
    for (auto parent = index.parent(); parent.isValid() && parent != rootIndex(); parent = parent.parent()) {
        ++levels;
    }
    return levels * indentation();
}

QRect DeckListTreeView::branchRect(const StickyHeader &sticky) const
{
    const int column = treeColumn();
    if ((!rootIsDecorated() && sticky.index.parent() == rootIndex()) || isColumnHidden(column)) {
        return {};
    }
    const int x = header()->sectionViewportPosition(column);
    const int indent = headerIndent(sticky.index);
    const int branchX = isRightToLeft() ? x + columnWidth(column) - indent : x + indent - indentation();
    return QRect(branchX, sticky.rect.y(), indentation(), sticky.rect.height())
        .intersected(QRect(x, sticky.rect.y(), columnWidth(column), sticky.rect.height()));
}

QModelIndex DeckListTreeView::indexAt(const QPoint &point) const
{
    const auto sticky = nativeHitTest ? StickyHeader{} : stickyHeader();
    if (sticky.index.isValid() && viewport()->rect().contains(point) && sticky.rect.contains(point)) {
        const int column = columnAt(point.x());
        return column < 0 ? QModelIndex() : sticky.index.siblingAtColumn(column);
    }
    return QTreeView::indexAt(point);
}

void DeckListTreeView::paintEvent(QPaintEvent *event)
{
    QTreeView::paintEvent(event);
    const auto sticky = stickyHeader();
    if (!sticky.index.isValid()) {
        return;
    }
    QPainter painter(viewport());
    painter.setClipRect(sticky.rect.intersected(viewport()->rect()));
    painter.fillRect(sticky.rect, palette().base());
    for (int column = 0; column < header()->count(); ++column) {
        if (isColumnHidden(column)) {
            continue;
        }
        const auto index = sticky.index.siblingAtColumn(column);
        QStyleOptionViewItem option;
        initViewItemOption(&option);
        option.rect = QRect(header()->sectionViewportPosition(column), sticky.rect.y(), columnWidth(column),
                            sticky.rect.height());
        option.state.setFlag(QStyle::State_Selected, selectionModel()->isSelected(index));
        option.state.setFlag(QStyle::State_HasFocus, hasFocus() && currentIndex() == index);
        const auto background = index.data(Qt::BackgroundRole);
        if (background.canConvert<QBrush>()) {
            painter.fillRect(option.rect, qvariant_cast<QBrush>(background));
        }
        if (column == treeColumn()) {
            const int indent = qMin(headerIndent(index), option.rect.width());
            if (isRightToLeft()) {
                option.rect.adjust(0, 0, -indent, 0);
            } else {
                option.rect.adjust(indent, 0, 0, 0);
            }
        }
        itemDelegateForIndex(index)->paint(&painter, option, index);
    }
    if (itemsExpandable() && model()->hasChildren(sticky.index)) {
        QStyleOption branch;
        branch.initFrom(this);
        branch.rect = branchRect(sticky);
        branch.state |= QStyle::State_Children | QStyle::State_Item;
        branch.state.setFlag(QStyle::State_Open, isExpanded(sticky.index));
        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &branch, &painter, this);
    }
    painter.setPen(palette().mid().color());
    painter.drawLine(sticky.rect.bottomLeft(), sticky.rect.bottomRight());
}

void DeckListTreeView::scrollContentsBy(int dx, int dy)
{
    QTreeView::scrollContentsBy(dx, dy);
    // Native scrolling copies viewport pixels; repaint so a pinned row cannot leave a ghost behind.
    viewport()->update();
}

void DeckListTreeView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    QTreeView::scrollTo(index, hint);
    const auto sticky = stickyHeader();
    const QRect rect = visualRect(index);
    if (sticky.index.isValid() && index.data(DeckRoles::IsCardRole).toBool() && rect.top() >= 0 &&
        rect.top() <= sticky.rect.bottom()) {
        const int offset = sticky.rect.bottom() + 1 - rect.top();
        const int steps = verticalScrollMode() == ScrollPerPixel ? offset : 1;
        verticalScrollBar()->setValue(verticalScrollBar()->value() - steps);
    }
}

void DeckListTreeView::forwardHeaderMouse(QMouseEvent *event)
{
    // Let QAbstractItemView maintain its own selection anchor, modifier state and release semantics.
    // Skip QTreeView's coordinate-based branch handling, which would act on the card under the strip.
    const bool autoScroll = hasAutoScroll();
    setAutoScroll(false);
    nativeHitTest = true;
    const QPointF point(event->position().x(), visualRect(pressedHeader).center().y());
    QMouseEvent translated(event->type(), point, event->globalPosition(), event->button(), event->buttons(),
                           event->modifiers());
    if (event->type() == QEvent::MouseButtonPress) {
        QAbstractItemView::mousePressEvent(&translated);
    } else {
        QAbstractItemView::mouseReleaseEvent(&translated);
    }
    nativeHitTest = false;
    setAutoScroll(autoScroll);
    event->accept();
}

void DeckListTreeView::mousePressEvent(QMouseEvent *event)
{
    const auto sticky = stickyHeader();
    headerPress = sticky.index.isValid() && sticky.rect.contains(event->position().toPoint());
    pressedHeader = QModelIndex();
    branchPress = false;
    if (!headerPress) {
        QTreeView::mousePressEvent(event);
        return;
    }
    pressedHeader = indexAt(event->position().toPoint());
    branchPress = event->button() == Qt::LeftButton && branchRect(sticky).contains(event->position().toPoint()) &&
                  itemsExpandable();
    if (branchPress) {
        setExpanded(sticky.index, !isExpanded(sticky.index));
        event->accept();
    } else {
        forwardHeaderMouse(event);
    }
}

void DeckListTreeView::mouseReleaseEvent(QMouseEvent *event)
{
    if (!headerPress) {
        QTreeView::mouseReleaseEvent(event);
        return;
    }
    if (!branchPress && pressedHeader.isValid() && indexAt(event->position().toPoint()) == pressedHeader) {
        forwardHeaderMouse(event);
    } else if (!branchPress) {
        QAbstractItemView::mouseReleaseEvent(event);
    }
    headerPress = false;
    // Retain the originating header through a possible double click, even if collapse removes the strip.
    event->accept();
}

void DeckListTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (pressedHeader.isValid()) {
        headerPress = true;
        if (!branchPress && event->button() == Qt::LeftButton && expandsOnDoubleClick() && itemsExpandable()) {
            const auto category = pressedHeader.sibling(pressedHeader.row(), 0);
            setExpanded(category, !isExpanded(category));
        }
        branchPress = true;
        event->accept();
        return;
    }
    QTreeView::mouseDoubleClickEvent(event);
}

void DeckListTreeView::mouseMoveEvent(QMouseEvent *event)
{
    if (headerPress) {
        event->accept();
    } else {
        QTreeView::mouseMoveEvent(event);
    }
}
