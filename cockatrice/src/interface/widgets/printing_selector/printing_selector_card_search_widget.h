/**
 * @file printing_selector_card_search_widget.h
 * @ingroup PrintingWidgets
 * @brief TODO: Document this.
 */

#ifndef PRINTING_SELECTOR_CARD_SEARCH_WIDGET_H
#define PRINTING_SELECTOR_CARD_SEARCH_WIDGET_H

#include "printing_selector.h"

#include <QLineEdit>
#include <QTimer>
#include <QWidget>

class PrintingSelectorCardSearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PrintingSelectorCardSearchWidget(PrintingSelector *parent);
    QString getSearchText();

private:
    QHBoxLayout *layout;
    PrintingSelector *parent;
    QLineEdit *searchBar;
    QTimer *searchDebounceTimer;
};

#endif // PRINTING_SELECTOR_CARD_SEARCH_WIDGET_H
