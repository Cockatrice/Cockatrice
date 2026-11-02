#ifndef DECK_LIST_TREE_VIEW_H
#define DECK_LIST_TREE_VIEW_H

#include <QPersistentModelIndex>
#include <QTreeView>

/** The shared deck table, with the current category kept visible while scrolling. */
class DeckListTreeView : public QTreeView
{
public:
    explicit DeckListTreeView(QWidget *parent = nullptr);
    void setModel(QAbstractItemModel *model) override;
    QModelIndex indexAt(const QPoint &point) const override;
    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void scrollContentsBy(int dx, int dy) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    struct StickyHeader
    {
        QModelIndex index;
        QRect rect;
    };
    StickyHeader stickyHeader() const;
    QRect branchRect(const StickyHeader &header) const;
    int treeColumn() const;
    int headerIndent(const QModelIndex &index) const;
    void forwardHeaderMouse(QMouseEvent *event);

    QList<QMetaObject::Connection> modelConnections;
    QPersistentModelIndex pressedHeader;
    bool headerPress = false;
    bool branchPress = false;
    bool nativeHitTest = false;
};

#endif
