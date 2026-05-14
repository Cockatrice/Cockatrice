/**
 * @file flow_layout.h
 * @ingroup UI
 * @brief A QLayout subclass that arranges child widgets in wrapping rows (horizontal flow)
 *        or wrapping columns (vertical flow).
 */

#ifndef FLOW_LAYOUT_H
#define FLOW_LAYOUT_H

#include <QLayout>
#include <QList>
#include <QLoggingCategory>
#include <QStyle>
#include <QWidget>

inline Q_LOGGING_CATEGORY(FlowLayoutLog, "flow_layout", QtInfoMsg);

class FlowLayout : public QLayout
{
public:
    explicit FlowLayout(QWidget *parent = nullptr);
    FlowLayout(QWidget *parent, Qt::Orientation flowDirection, int margin = 0, int hSpacing = 0, int vSpacing = 0);
    ~FlowLayout() override;

    void insertWidgetAtIndex(QWidget *toInsert, int index);

    // QLayout interface
    void addItem(QLayoutItem *item) override;
    [[nodiscard]] int count() const override;
    [[nodiscard]] QLayoutItem *itemAt(int index) const override;
    QLayoutItem *takeAt(int index) override;
    void setGeometry(const QRect &rect) override;

    // Size negotiation
    [[nodiscard]] Qt::Orientations expandingDirections() const override;
    [[nodiscard]] bool hasHeightForWidth() const override;
    [[nodiscard]] int heightForWidth(int width) const override;
    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSize() const override;

    // Spacing helpers
    void setHorizontalMargin(int margin)
    {
        horizontalMargin = margin;
    }
    [[nodiscard]] int horizontalSpacing() const;
    void setVerticalMargin(int margin)
    {
        verticalMargin = margin;
    }
    [[nodiscard]] int verticalSpacing() const;
    [[nodiscard]] int smartSpacing(QStyle::PixelMetric pm) const;

    // Layout passes (virtual so subclasses can override placement logic)
    virtual int layoutAllRows(int originX, int originY, int availableWidth);
    virtual void layoutSingleRow(const QVector<QLayoutItem *> &rowItems, int x, int y, int availableWidth);
    int layoutAllColumns(int originX, int originY, int availableHeight);
    void layoutSingleColumn(const QVector<QLayoutItem *> &colItems, int x, int y);

protected:
    // Size-hint helpers split by direction
    [[nodiscard]] QSize calculateSizeHintHorizontal() const;
    [[nodiscard]] QSize calculateMinimumSizeHorizontal() const;
    [[nodiscard]] QSize calculateSizeHintVertical() const;
    [[nodiscard]] QSize calculateMinimumSizeVertical() const;

    QList<QLayoutItem *> items;
    Qt::Orientation flowDirection;
    int horizontalMargin; ///< Horizontal spacing between items (-1 = use style default)
    int verticalMargin;   ///< Vertical spacing between items  (-1 = use style default)
};

#endif // FLOW_LAYOUT_H