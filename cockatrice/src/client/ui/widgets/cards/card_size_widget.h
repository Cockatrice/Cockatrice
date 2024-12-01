#ifndef CARD_SIZE_WIDGET_H
#define CARD_SIZE_WIDGET_H

#include "../general/layout_containers/flow_widget.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>

class CardSizeWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit CardSizeWidget(QWidget *parent, FlowWidget* flowWidget = nullptr);
        QSlider* getSlider() const;

private:
    QWidget *parent;
    FlowWidget* flowWidget;
    QHBoxLayout *cardSizeLayout;
    QLabel *cardSizeLabel;
    QSlider *cardSizeSlider;
};

#endif //CARD_SIZE_WIDGET_H
