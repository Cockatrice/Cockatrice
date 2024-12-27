#ifndef PRINTING_SELECTOR_VIEW_OPTIONS_WIDGET_H
#define PRINTING_SELECTOR_VIEW_OPTIONS_WIDGET_H

#include "../general/layout_containers/flow_widget.h"
#include "printing_selector.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QWidget>

class PrintingSelectorViewOptionsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PrintingSelectorViewOptionsWidget(QWidget *parent, PrintingSelector *_printingSelector);

private:
    QGridLayout *gridLayout;
    PrintingSelector *printingSelector;
    QCheckBox *sortCheckBox;
    QCheckBox *searchCheckBox;
    QCheckBox *cardSizeCheckBox;
    QCheckBox *navigationCheckBox;
};

#endif // PRINTING_SELECTOR_VIEW_OPTIONS_WIDGET_H
