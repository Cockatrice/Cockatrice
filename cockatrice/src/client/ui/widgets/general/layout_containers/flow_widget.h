#ifndef FLOW_WIDGET_H
#define FLOW_WIDGET_H
#include "../../../layouts/flow_layout.h"

#include <QHBoxLayout>
#include <QWidget>
#include <qscrollarea.h>

class FlowWidget : public QWidget
{
    Q_OBJECT

public:
    FlowWidget(QWidget *parent, Qt::ScrollBarPolicy hPolicy, Qt::ScrollBarPolicy vPolicy);
    void addWidget(QWidget *widget_to_add) const;
    void clearLayout();

    QScrollArea *scrollArea;

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QHBoxLayout *main_layout;
    FlowLayout *flow_layout;
    QWidget *container;
};

#endif // FLOW_WIDGET_H
