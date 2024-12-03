#ifndef PRINTING_SELECTOR_SORT_AND_SEARCH_TOOLBAR_WIDGET_H
#define PRINTING_SELECTOR_SORT_AND_SEARCH_TOOLBAR_WIDGET_H

#include "printing_selector.h"
#include "printing_selector_view_options_widget.h"

#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>

class PrintingSelectorViewOptionsToolbarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PrintingSelectorViewOptionsToolbarWidget(QWidget *parent, PrintingSelector *printingSelector);
    void collapse();
    void expand();
    void onWidgetChanged(int);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    PrintingSelectorViewOptionsWidget *getViewOptionsWidget() const;

private:
    QVBoxLayout *layout;
    PrintingSelector *printingSelector;
    PrintingSelectorViewOptionsWidget *viewOptions;
    QWidget *expandedWidget;
    QPushButton *collapseButton;
    QWidget *collapsedWidget;
    QPushButton *expandButton;
    QStackedWidget *stackedWidget;
};

#endif // PRINTING_SELECTOR_SORT_AND_SEARCH_TOOLBAR_WIDGET_H
