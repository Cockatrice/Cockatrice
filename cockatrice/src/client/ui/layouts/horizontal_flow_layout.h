#ifndef HORIZONTAL_FLOW_LAYOUT_H
#define HORIZONTAL_FLOW_LAYOUT_H

#include "flow_layout.h"

class HorizontalFlowLayout : public FlowLayout
{
public:
    explicit HorizontalFlowLayout(QWidget *parent = nullptr);
    ~HorizontalFlowLayout() override;

    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;

    void setGeometry(const QRect &rect) override;
    int layoutRows(int originX, int originY, int availableWidth) override;
    void layoutRow(const QVector<QLayoutItem *> &rowItems, int x, int y) override;
};

#endif // HORIZONTAL_FLOW_LAYOUT_H