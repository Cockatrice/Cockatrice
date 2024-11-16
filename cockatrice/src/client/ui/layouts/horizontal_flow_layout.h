#ifndef HORIZONTAL_FLOW_LAYOUT_H
#define HORIZONTAL_FLOW_LAYOUT_H

#include "flow_layout.h"

class HorizontalFlowLayout : public FlowLayout
{
public:
    explicit HorizontalFlowLayout(QWidget *parent = nullptr, int margin = 0, int hSpacing = 0, int vSpacing = 0);
    ~HorizontalFlowLayout() override;

    int heightForWidth(int width) const override;

    void setGeometry(const QRect &rect) override;
    int layoutAllColumns(int originX, int originY, int availableWidth);
    void layoutSingleColumn(const QVector<QLayoutItem *> &rowItems, int x, int y);
};

#endif // HORIZONTAL_FLOW_LAYOUT_H