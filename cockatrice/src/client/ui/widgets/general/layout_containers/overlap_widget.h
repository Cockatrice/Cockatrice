#ifndef OVERLAP_WIDGET_H
#define OVERLAP_WIDGET_H

#include "../../../layouts/overlap_layout.h"

#include <QHBoxLayout>
#include <QWidget>

class OverlapWidget : public QWidget
{
    Q_OBJECT

public:
    OverlapWidget(int overlapPercentage, int maxColumns, int maxRows, Qt::Orientation direction, QWidget *parent);
    void addWidget(QWidget *widget_to_add);
    void clearLayout();

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

};


#endif //OVERLAP_WIDGET_H
