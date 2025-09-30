/**
 * @file visual_database_display_sub_type_filter_widget.h
 * @ingroup VisualCardDatabaseWidgets
 * @brief TODO: Document this.
 */

#ifndef VISUAL_DATABASE_DISPLAY_SUB_TYPE_FILTER_WIDGET_H
#define VISUAL_DATABASE_DISPLAY_SUB_TYPE_FILTER_WIDGET_H

#include "../../../filters/filter_tree_model.h"
#include "../general/layout_containers/flow_widget.h"

#include <QMap>
#include <QPushButton>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

class VisualDatabaseDisplaySubTypeFilterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VisualDatabaseDisplaySubTypeFilterWidget(QWidget *parent, FilterTreeModel *filterModel);
    void retranslateUi();
    void createSubTypeButtons();
    void updateSubTypeButtonsVisibility();
    int getMaxSubTypeCount() const;

    void handleSubTypeToggled(const QString &mainType, bool active);
    void updateSubTypeFilter();
    void updateFilterMode(bool checked);
    void syncWithFilterModel();

private:
    FilterTreeModel *filterModel;
    QMap<QString, int> allSubCardTypesWithCount;
    QSpinBox *spinBox;
    QVBoxLayout *layout;
    QLineEdit *searchBox;
    FlowWidget *flowWidget;
    QPushButton *toggleButton; // Mode switch button

    QMap<QString, bool> activeSubTypes;       // Track active filters
    QMap<QString, QPushButton *> typeButtons; // Store toggle buttons

    bool exactMatchMode = false; // Toggle between "Exact Match" and "Includes"
};

#endif // VISUAL_DATABASE_DISPLAY_SUB_TYPE_FILTER_WIDGET_H
