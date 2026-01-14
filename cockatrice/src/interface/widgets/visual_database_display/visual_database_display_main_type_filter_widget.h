/**
 * @file visual_database_display_main_type_filter_widget.h
 * @ingroup VisualCardDatabaseWidgets
 * @brief TODO: Document this.
 */

#ifndef VISUAL_DATABASE_DISPLAY_MAIN_TYPE_FILTER_WIDGET_H
#define VISUAL_DATABASE_DISPLAY_MAIN_TYPE_FILTER_WIDGET_H

#include "../../../filters/filter_tree_model.h"
#include "../general/layout_containers/flow_widget.h"

#include <QLabel>
#include <QMap>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

class VisualDatabaseDisplayMainTypeFilterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VisualDatabaseDisplayMainTypeFilterWidget(QWidget *parent, FilterTreeModel *filterModel);
    void retranslateUi();
    void createMainTypeButtons();
    void updateMainTypeButtonsVisibility();
    int getMaxMainTypeCount() const;

    void handleMainTypeToggled(const QString &mainType, bool active);
    void updateMainTypeFilter();
    void updateFilterMode();
    void syncWithFilterModel();

private:
    FilterTreeModel *filterModel;
    QMap<QString, int> allMainCardTypesWithCount;

    QVBoxLayout *layout;
    FlowWidget *flowWidget;
    QLabel *thresholdLabel;
    QSpinBox *spinBox;
    QPushButton *toggleButton; // Mode switch button

    QMap<QString, bool> activeMainTypes;      // Track active filters
    QMap<QString, QPushButton *> typeButtons; // Store toggle buttons

    bool exactMatchMode = true; // Toggle between "Exact Match" and "Includes"
};

#endif // VISUAL_DATABASE_DISPLAY_MAIN_TYPE_FILTER_WIDGET_H
