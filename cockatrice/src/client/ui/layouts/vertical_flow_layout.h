#ifndef VERTICAL_FLOW_LAYOUT_H
#define VERTICAL_FLOW_LAYOUT_H

#include "flow_layout.h"

class VerticalFlowLayout : public FlowLayout
{
public:
    explicit VerticalFlowLayout(QWidget *parent = nullptr, int margin = 0, int hSpacing = 0, int vSpacing = 0);
    ~VerticalFlowLayout() override;

    [[nodiscard]] int heightForWidth(int width) const override;

    void setGeometry(const QRect &rect) override;
    int layoutAllRows(int originX, int originY, int availableWidth) override;
    void layoutSingleRow(const QVector<QLayoutItem *> &rowItems, int x, int y) override;
};

#endif // VERTICAL_FLOW_LAYOUT_H