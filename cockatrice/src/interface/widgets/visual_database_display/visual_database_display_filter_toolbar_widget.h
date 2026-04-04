#ifndef COCKATRICE_VISUAL_DATABASE_DISPLAY_FILTER_TOOLBAR_WIDGET_H
#define COCKATRICE_VISUAL_DATABASE_DISPLAY_FILTER_TOOLBAR_WIDGET_H

#include <QWidget>

class VisualDatabaseDisplayWidget;
class QGroupBox;
class QComboBox;
class QHBoxLayout;
class QLabel;
class SettingsButtonWidget;
class VisualDatabaseDisplayFilterSaveLoadWidget;
class VisualDatabaseDisplayMainTypeFilterWidget;
class VisualDatabaseDisplayNameFilterWidget;
class VisualDatabaseDisplaySubTypeFilterWidget;
class VisualDatabaseDisplaySetFilterWidget;
class VisualDatabaseDisplayFormatLegalityFilterWidget;
class VisualDatabaseDisplayFilterToolbarWidget : public QWidget
{
    Q_OBJECT

signals:
    void searchModelChanged();

public:
    explicit VisualDatabaseDisplayFilterToolbarWidget(VisualDatabaseDisplayWidget *parent);
    void initialize();
    void retranslateUi();

private:
    VisualDatabaseDisplayWidget *visualDatabaseDisplay;

    QGroupBox *sortGroupBox;
    QLabel *sortByLabel;
    QComboBox *sortColumnCombo, *sortOrderCombo;

    QGroupBox *filterGroupBox;
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
