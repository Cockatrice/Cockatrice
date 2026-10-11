#ifndef COCKATRICE_VISUAL_DATABASE_DISPLAY_FILTER_TOOLBAR_WIDGET_H
#define COCKATRICE_VISUAL_DATABASE_DISPLAY_FILTER_TOOLBAR_WIDGET_H

#include "../general/layout_containers/flow_widget.h"

#include <qtmetamacros.h>

class VisualDatabaseDisplayWidget;
class DeckListModel;
class QComboBox;
class QGroupBox;
class QLabel;
class SettingsButtonWidget;
class VisualDatabaseDisplayFilterSaveLoadWidget;
class VisualDatabaseDisplayFormatLegalityFilterWidget;
class VisualDatabaseDisplayMainTypeFilterWidget;
class VisualDatabaseDisplayNameFilterWidget;
class VisualDatabaseDisplaySetFilterWidget;
class VisualDatabaseDisplaySubTypeFilterWidget;

class VisualDatabaseDisplayFilterToolbarWidget : public FlowWidget
{
    Q_OBJECT

signals:
    void searchModelChanged();

public:
    explicit VisualDatabaseDisplayFilterToolbarWidget(VisualDatabaseDisplayWidget *parent,
                                                      DeckListModel *deckListModel = nullptr);
    void initialize();
    void retranslateUi();

private:
    VisualDatabaseDisplayWidget *visualDatabaseDisplay;
    DeckListModel *deckListModel;

    QGroupBox *sortGroupBox;
    QLabel *sortByLabel;
    QComboBox *sortColumnCombo, *sortOrderCombo;

    QGroupBox *filterGroupBox;
    QLabel *filterByLabel;

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

    int fullWidthHint = 0;
    void updateCompactMode(int availableWidth);

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // COCKATRICE_VISUAL_DATABASE_DISPLAY_FILTER_TOOLBAR_WIDGET_H
