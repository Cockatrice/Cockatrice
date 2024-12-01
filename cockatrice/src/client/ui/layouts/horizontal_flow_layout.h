#ifndef HORIZONTAL_FLOW_LAYOUT_H
#define HORIZONTAL_FLOW_LAYOUT_H

#include "flow_layout.h"

class HorizontalFlowLayout : public FlowLayout
{
public:
    explicit HorizontalFlowLayout(QWidget *parent = nullptr, int margin = 0, int hSpacing = 0, int vSpacing = 0);
    ~HorizontalFlowLayout() override;

    [[nodiscard]] int heightForWidth(int height) const override;

    void setGeometry(const QRect &rect) override;
    int layoutAllColumns(int originX, int originY, int availableHeight);
    static void layoutSingleColumn(const QVector<QLayoutItem *> &colItems, int x, int y);
};

#endif // HORIZONTAL_FLOW_LAYOUT_H