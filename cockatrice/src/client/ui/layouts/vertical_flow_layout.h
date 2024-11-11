#ifndef VERTICAL_FLOW_LAYOUT_H
#define VERTICAL_FLOW_LAYOUT_H

#include "flow_layout.h"

class VerticalFlowLayout : public FlowLayout
{
public:
    explicit VerticalFlowLayout(QWidget *parent = nullptr, int margin = 0, int hSpacing = 0, int vSpacing = 0);
    ~VerticalFlowLayout() override;

    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;

    void setGeometry(const QRect &rect) override;
    int layoutRows(int originX, int originY, int availableWidth);
    void layoutRow(const QVector<QLayoutItem *> &rowItems, int x, int y);
};

#endif // VERTICAL_FLOW_LAYOUT_H