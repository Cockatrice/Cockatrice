#ifndef OVERLAP_LAYOUT_H
#define OVERLAP_LAYOUT_H

#include <QLayout>
#include <QList>
#include <QWidget>


class OverlapLayout : public QLayout {
public:
    OverlapLayout(int overlapPercentage = 10, int maxColumns = 2, int maxRows = 2, Qt::Orientation direction = Qt::Horizontal, QWidget* parent = nullptr);
    ~OverlapLayout();

    void addItem(QLayoutItem* item) override;
    int count() const override;
    QLayoutItem* itemAt(int index) const override;
    QLayoutItem* takeAt(int index) override;
    void setGeometry(const QRect& rect) override;
    QSize minimumSize() const override;
    QSize sizeHint() const override;
    void setMaxColumns(int newValue);
    void setMaxRows(int newValue);
    int calculateMaxColumns() const;
    int calculateRowsForColumns(int columns) const;
    int calculateMaxRows() const;
    int calculateColumnsForRows(int rows) const;
    void setDirection(Qt::Orientation direction);

private:
    QList<QLayoutItem*> itemList;
    int overlapPercentage;
    int maxColumns;
    int maxRows;
    Qt::Orientation direction;

    // Calculate the preferred size of the layout
    QSize calculatePreferredSize() const;
};

#endif // OVERLAP_LAYOUT_H
