/**
 * @file flow_layout.h
 * @ingroup UI
 * @brief TODO: Document this.
 */

#ifndef FLOW_LAYOUT_H
#define FLOW_LAYOUT_H

#include <QLayout>
#include <QList>
#include <QLoggingCategory>
#include <QWidget>
#include <qstyle.h>

inline Q_LOGGING_CATEGORY(FlowLayoutLog, "flow_layout", QtInfoMsg);

class FlowLayout : public QLayout
{
public:
    explicit FlowLayout(QWidget *parent = nullptr);
    FlowLayout(QWidget *parent, Qt::Orientation _flowDirection, int margin = 0, int hSpacing = 0, int vSpacing = 0);
    ~FlowLayout() override;
    void insertWidgetAtIndex(QWidget *toInsert, int index);

    QSize calculateMinimumSizeHorizontal() const;
    QSize calculateSizeHintVertical() const;
    QSize calculateMinimumSizeVertical() const;
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
    int layoutAllColumns(int originX, int originY, int availableHeight);
    void layoutSingleColumn(const QVector<QLayoutItem *> &colItems, int x, int y);
    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSize() const override;
    QSize calculateSizeHintHorizontal() const;

protected:
    QList<QLayoutItem *> items; // List to store layout items
    Qt::Orientation flowDirection;
    int horizontalMargin;
    int verticalMargin;
};

#endif // FLOW_LAYOUT_H