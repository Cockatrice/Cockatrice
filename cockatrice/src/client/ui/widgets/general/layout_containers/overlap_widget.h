#ifndef OVERLAP_WIDGET_H
#define OVERLAP_WIDGET_H

#include "../../../layouts/overlap_layout.h"

#include <QHBoxLayout>
#include <QWidget>

class OverlapWidget : public QWidget
{
    Q_OBJECT

public:
    OverlapWidget(QWidget *parent, int overlapPercentage, int maxColumns, int maxRows, Qt::Orientation direction, bool adjustOnResize = false);
    void addWidget(QWidget *widget_to_add);
    void clearLayout();
    void adjustMaxColumnsAndRows();

public slots:
    void maxOverlapItemsChanged(int newValue);
    void overlapDirectionChanged(QString newDirection);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    OverlapLayout *overlap_layout;
    int overlapPercentage;
    int maxColumns;
    int maxRows;
    Qt::Orientation direction;
    bool adjustOnResize = false;

};


#endif //OVERLAP_WIDGET_H
