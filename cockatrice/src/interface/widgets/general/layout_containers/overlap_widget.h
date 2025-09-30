/**
 * @file overlap_widget.h
 * @ingroup UI
 * @brief TODO: Document this.
 */

#ifndef OVERLAP_WIDGET_H
#define OVERLAP_WIDGET_H

#include "../../../layouts/overlap_layout.h"

#include <QWidget>

class OverlapWidget final : public QWidget
{
    Q_OBJECT

public:
    OverlapWidget(QWidget *parent,
                  int overlapPercentage,
                  int maxColumns,
                  int maxRows,
                  Qt::Orientation direction,
                  bool adjustOnResize = false);
    void addWidget(QWidget *widgetToAdd) const;
    void insertWidgetAtIndex(QWidget *toInsert, int index);
    void removeWidget(QWidget *widgetToRemove) const;
    void clearLayout();
    void adjustMaxColumnsAndRows();

public slots:
    void maxOverlapItemsChanged(int newValue);
    void overlapDirectionChanged(const QString &newDirection);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    OverlapLayout *overlapLayout;
    int overlapPercentage;
    int maxColumns;
    int maxRows;
    Qt::Orientation direction;
    bool adjustOnResize = false;
};

#endif // OVERLAP_WIDGET_H
