/**
 * @file printing_selector_card_search_widget.h
 * @ingroup PrintingWidgets
 */
//! \todo Document this file.

#ifndef PRINTING_SELECTOR_CARD_SEARCH_WIDGET_H
#define PRINTING_SELECTOR_CARD_SEARCH_WIDGET_H

#include <QString>
#include <QWidget>
#include <qtmetamacros.h>

class PrintingSelector;
class QHBoxLayout;
class QLineEdit;
class QTimer;

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
