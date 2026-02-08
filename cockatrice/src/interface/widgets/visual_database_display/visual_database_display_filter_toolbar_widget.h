#ifndef COCKATRICE_VISUAL_DATABASE_DISPLAY_FILTER_TOOLBAR_WIDGET_H
#define COCKATRICE_VISUAL_DATABASE_DISPLAY_FILTER_TOOLBAR_WIDGET_H

#include "visual_database_display_filter_save_load_widget.h"
#include "visual_database_display_format_legality_filter_widget.h"
#include "visual_database_display_main_type_filter_widget.h"
#include "visual_database_display_name_filter_widget.h"
#include "visual_database_display_set_filter_widget.h"
#include "visual_database_display_sub_type_filter_widget.h"

class VisualDatabaseDisplayWidget;

class VisualDatabaseDisplayFilterToolbarWidget : public QWidget
{
    Q_OBJECT

signals:
    void searchModelChanged();

public:
    explicit VisualDatabaseDisplayFilterToolbarWidget(VisualDatabaseDisplayWidget *parent);
    void initialize();
    void retranslateUi();

    SettingsButtonWidget *getSetFilterWidget()
    {
        return quickFilterSetWidget;
    };

private:
    VisualDatabaseDisplayWidget *visualDatabaseDisplay;

    QLabel *sortByLabel;
    QComboBox *sortColumnCombo, *sortOrderCombo;

    QLabel *filterByLabel;

    QHBoxLayout *filterContainerLayout;
    SettingsButtonWidget *quickFilterSaveLoadWidget;
    VisualDatabaseDisplayFilterSaveLoadWidget *saveLoadWidget;
    SettingsButtonWidget *quickFilterNameWidget;
    VisualDatabaseDisplayNameFilterWidget *nameFilterWidget;
    SettingsButtonWidget *quickFilterMainTypeWidget;
    VisualDatabaseDisplayMainTypeFilterWidget *mainTypeFilterWidget;
    SettingsButtonWidget *quickFilterSubTypeWidget;
    VisualDatabaseDisplaySubTypeFilterWidget *subTypeFilterWidget;
    SettingsButtonWidget *quickFilterSetWidget;
    VisualDatabaseDisplaySetFilterWidget *setFilterWidget;
    SettingsButtonWidget *quickFilterFormatLegalityWidget;
    VisualDatabaseDisplayFormatLegalityFilterWidget *formatLegalityWidget;
};

#endif // COCKATRICE_VISUAL_DATABASE_DISPLAY_FILTER_TOOLBAR_WIDGET_H
