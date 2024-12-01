#ifndef PRINTING_SELECTOR_CARD_SELECTION_WIDGET_H
#define PRINTING_SELECTOR_CARD_SELECTION_WIDGET_H

#include "printing_selector.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>

class PrintingSelectorCardSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PrintingSelectorCardSelectionWidget(PrintingSelector *parent);

    void connectSignals();

private:
    PrintingSelector *parent;
    QHBoxLayout *cardSelectionBarLayout;
    QPushButton *previousCardButton;
    QPushButton *nextCardButton;
};

#endif // PRINTING_SELECTOR_CARD_SELECTION_WIDGET_H
