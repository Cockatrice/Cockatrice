/**
 * @file visual_database_display_name_filter_widget.h
 * @ingroup VisualCardDatabaseWidgets
 */
//! \todo Document this file.

#ifndef VISUAL_DATABASE_DISPLAY_NAME_FILTER_WIDGET_H
#define VISUAL_DATABASE_DISPLAY_NAME_FILTER_WIDGET_H

#include "../../../filters/filter_tree_model.h"
#include "../../../interface/widgets/tabs/abstract_tab_deck_editor.h"
#include "../general/layout_containers/flow_widget.h"

#include <QLineEdit>
#include <QMap>
#include <QPushButton>
#include <QWidget>

class VisualDatabaseDisplayNameFilterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VisualDatabaseDisplayNameFilterWidget(QWidget *parent,
                                                   FilterTreeModel *filterModel,
                                                   DeckListModel *deckListModel = nullptr);

    void createNameFilter(const QString &name);
    void removeNameFilter(const QString &name);
    void updateFilterList();
    void updateFilterModel();
    void syncWithFilterModel();

public slots:
    void retranslateUi();

private:
    FilterTreeModel *filterModel;
    DeckListModel *deckListModel;
    QVBoxLayout *layout;
    QLineEdit *searchBox;
    FlowWidget *flowWidget;
    QPushButton *loadFromDeckButton;
    QPushButton *loadFromClipboardButton;

    QMap<QString, QPushButton *> activeFilters; // Store active name filter buttons

private slots:
    void actLoadFromDeck();
    void actLoadFromClipboard();
};

#endif // VISUAL_DATABASE_DISPLAY_NAME_FILTER_WIDGET_H
