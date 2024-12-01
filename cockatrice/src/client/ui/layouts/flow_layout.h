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
    ~FlowLayout() override;

    void addItem(QLayoutItem *item) override;
    [[nodiscard]] int count() const override;
    [[nodiscard]] QLayoutItem *itemAt(int index) const override;
    QLayoutItem *takeAt(int index) override;
    [[nodiscard]] int horizontalSpacing() const;

    [[nodiscard]] Qt::Orientations expandingDirections() const override;
    [[nodiscard]] bool hasHeightForWidth() const override;
    [[nodiscard]] int heightForWidth(int width) const override;
    [[nodiscard]] int verticalSpacing() const;
    [[nodiscard]] int doLayout(const QRect &rect, bool testOnly) const;
    [[nodiscard]] int smartSpacing(QStyle::PixelMetric pm) const;
    [[nodiscard]] int getParentScrollAreaWidth() const;
    [[nodiscard]] int getParentScrollAreaHeight() const;

    void setGeometry(const QRect &rect) override;
    virtual int layoutAllRows(int originX, int originY, int availableWidth);
    virtual void layoutSingleRow(const QVector<QLayoutItem *> &rowItems, int x, int y);
    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSize() const override;

protected:
    QList<QLayoutItem *> items; // List to store layout items
    int horizontalMargin;
    int verticalMargin;
};

#endif // FLOW_LAYOUT_H