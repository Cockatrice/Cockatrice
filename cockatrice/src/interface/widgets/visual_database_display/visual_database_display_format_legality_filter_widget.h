#ifndef COCKATRICE_VISUAL_DATABASE_DISPLAY_FORMAT_LEGALITY_FILTER_WIDGET_H
#define COCKATRICE_VISUAL_DATABASE_DISPLAY_FORMAT_LEGALITY_FILTER_WIDGET_H

#include "../../../filters/filter_tree_model.h"
#include "../general/layout_containers/flow_widget.h"

#include <QMap>
#include <QPushButton>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

class VisualDatabaseDisplayFormatLegalityFilterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VisualDatabaseDisplayFormatLegalityFilterWidget(QWidget *parent, FilterTreeModel *filterModel);
    void retranslateUi();
    void createFormatButtons();
    void updateFormatButtonsVisibility();
    int getMaxMainTypeCount() const;

    void handleFormatToggled(const QString &format, bool active);
    void updateFormatFilter();
    void updateFilterMode(bool checked);
    void syncWithFilterModel();

private:
    FilterTreeModel *filterModel;
    QMap<QString, int> allFormatsWithCount;
    QSpinBox *spinBox;
    QHBoxLayout *layout;
    FlowWidget *flowWidget;
    QPushButton *toggleButton; // Mode switch button

    QMap<QString, bool> activeFormats;          // Track active filters
    QMap<QString, QPushButton *> formatButtons; // Store toggle buttons

    bool exactMatchMode = false; // Toggle between "Exact Match" and "Includes"
};

#endif // COCKATRICE_VISUAL_DATABASE_DISPLAY_FORMAT_LEGALITY_FILTER_WIDGET_H
