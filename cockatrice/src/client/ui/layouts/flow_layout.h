#ifndef FLOW_LAYOUT_H
#define FLOW_LAYOUT_H

#include <QLayout>
#include <QList>
#include <QWidget>
#include <qstyle.h>

class FlowLayout : public QLayout
{
public:
    explicit FlowLayout(QWidget *parent = nullptr);
    FlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing);
    ~FlowLayout();

    void addItem(QLayoutItem *item) override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QLayoutItem *takeAt(int index) override;
    int horizontalSpacing() const;

    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;
    int verticalSpacing() const;
    int doLayout(const QRect &rect, bool testOnly) const;
    int smartSpacing(QStyle::PixelMetric pm) const;
    int getParentScrollAreaWidth() const;
    int getParentScrollAreaHeight() const;

    void setGeometry(const QRect &rect) override;
    virtual int layoutRows(int originX, int originY, int availableWidth);
    virtual void layoutRow(const QVector<QLayoutItem *> &rowItems, int x, int y);
    QSize sizeHint() const override;
    QSize minimumSize() const override;

protected:
    QList<QLayoutItem *> items; // List to store layout items
    int m_hSpace;
    int m_vSpace;
};

#endif // FLOWLAYOUT_H