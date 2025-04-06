#ifndef CARD_SIZE_WIDGET_H
#define CARD_SIZE_WIDGET_H

#include "../general/layout_containers/flow_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QTimer>
#include <QWidget>

class CardSizeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CardSizeWidget(QWidget *parent, FlowWidget *flowWidget = nullptr, int defaultValue = 100);
    [[nodiscard]] QSlider *getSlider() const;

public slots:
    void updateCardSizeSetting(int newValue);

private:
    QWidget *parent;
    FlowWidget *flowWidget;
    QHBoxLayout *cardSizeLayout;
    QLabel *cardSizeLabel;
    QSlider *cardSizeSlider;
    QTimer debounceTimer; // Debounce timer
    int pendingValue;     // Stores the latest slider value
};

#endif // CARD_SIZE_WIDGET_H
