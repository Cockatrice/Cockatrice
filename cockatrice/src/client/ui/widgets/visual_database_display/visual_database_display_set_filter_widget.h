#ifndef VISUAL_DATABASE_DISPLAY_SET_FILTER_WIDGET_H
#define VISUAL_DATABASE_DISPLAY_SET_FILTER_WIDGET_H

#include "../../../../game/filters/filter_tree_model.h"
#include "../general/layout_containers/flow_widget.h"

#include <QLineEdit>
#include <QMap>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

class VisualDatabaseDisplaySetFilterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VisualDatabaseDisplaySetFilterWidget(QWidget *parent, FilterTreeModel *filterModel);
    void createSetButtons();
    void updateSetButtonsVisibility();
    void handleSetToggled(const QString &setShortName, bool active);

    void updateSetFilter();
    void updateFilterMode(bool checked);

private:
    FilterTreeModel *filterModel;
    QMap<QString, int> allMainCardTypesWithCount;
    QVBoxLayout *layout;
    QLineEdit *searchBox;
    FlowWidget *flowWidget;
    QPushButton *toggleButton; // Mode switch button

    QMap<QString, bool> activeMainTypes;      // Track active filters
    QMap<QString, QPushButton *> typeButtons; // Store toggle buttons
    QMap<QString, QPushButton *> setButtons;  // Store set filter buttons
    QMap<QString, bool> activeSets;           // Track active set filters

    bool exactMatchMode = false; // Toggle between "Exact Match" and "Includes"
};

#endif // VISUAL_DATABASE_DISPLAY_SET_FILTER_WIDGET_H
