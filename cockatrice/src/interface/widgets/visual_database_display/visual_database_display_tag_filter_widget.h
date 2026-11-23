#ifndef COCKATRICE_VISUAL_DATABASE_DISPLAY_TAG_FILTER_WIDGET_H
#define COCKATRICE_VISUAL_DATABASE_DISPLAY_TAG_FILTER_WIDGET_H

#include "../../../filters/filter_tree_model.h"
#include "../general/layout_containers/flow_widget.h"

#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

class VisualDatabaseDisplayTagFilterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VisualDatabaseDisplayTagFilterWidget(QWidget *parent, FilterTreeModel *filterModel);
    void retranslateUi();
    void createTagButtons();
    void updateTagButtonsVisibility();
    [[nodiscard]] int getMaxTagCount() const;

    void handleTagToggled(const QString &tag, bool active);
    void updateTagFilter();
    void updateFilterMode();
    void syncWithFilterModel();

private:
    FilterTreeModel *filterModel;
    QMap<QString, int> allTagsWithCount;

    QVBoxLayout *layout;
    QLineEdit *searchBox;
    FlowWidget *flowWidget;
    QLabel *thresholdLabel;
    QSpinBox *spinBox;
    QPushButton *toggleButton; // Mode switch button

    QMap<QString, bool> activeTags;          // Track active filters
    QMap<QString, QPushButton *> tagButtons; // Store toggle buttons

    bool exactMatchMode = true; // Toggle between "Exact Match" (all) and "Includes" (any)
};

#endif // COCKATRICE_VISUAL_DATABASE_DISPLAY_TAG_FILTER_WIDGET_H
