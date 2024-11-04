#ifndef VERTICAL_FLOW_LAYOUT_H
#define VERTICAL_FLOW_LAYOUT_H

#include "flow_layout.h"

class VerticalFlowLayout : public FlowLayout
{
public:
    explicit VerticalFlowLayout(QWidget *parent = nullptr);
    ~VerticalFlowLayout() override;

    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;

    void setGeometry(const QRect &rect) override;
    int layoutRows(int originX, int originY, int availableWidth) override;
    void layoutRow(const QVector<QLayoutItem *> &rowItems, int x, int y) override;
};

#endif // VERTICAL_FLOW_LAYOUT_H