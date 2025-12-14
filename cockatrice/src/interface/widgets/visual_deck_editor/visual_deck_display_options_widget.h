#ifndef COCKATRICE_VISUAL_DECK_DISPLAY_OPTIONS_WIDGET_H
#define COCKATRICE_VISUAL_DECK_DISPLAY_OPTIONS_WIDGET_H

#include "visual_deck_editor_widget.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QWidget>

/**
 * @class VisualDeckDisplayOptionsWidget
 * @brief A widget that controls how deck cards are displayed in the visual deck editor.
 *
 * This widget provides:
 *  - A **group-by** selector (QComboBox)
 *  - A **sort-by** multi-criteria, draggable list (QListWidget within a SettingsButtonWidget)
 *  - A **display-type toggler** (flat vs. overlap layout)
 *
 * Depending on whether the parent is a VisualDeckEditorWidget, this widget can mirror the
 * original group by checkbox from the main deck editor UI to maintain synchronization.
 *
 * It emits signals whenever the grouping criterion, sorting criteria, or display mode changes.
 */
class VisualDeckDisplayOptionsWidget : public QWidget
{
    Q_OBJECT
signals:
    /**
     * @brief Emitted when the display type (flat or overlapping layout) changes.
     * @param displayType The newly selected display layout.
     */
    void displayTypeChanged(const DisplayType &displayType);

    /**
     * @brief Emitted when a new grouping criterion is selected.
     * @param activeGroupCriteria Name of the selected group-by criterion.
     */
    void groupCriteriaChanged(const QString &activeGroupCriteria);

    /**
     * @brief Emitted when the order of sort criteria changes.
     * @param activeSortCriteria Ordered list of sorting keys.
     */
    void sortCriteriaChanged(const QStringList &activeSortCriteria);

public slots:
    /**
     * @brief Updates all UI text for retranslation/localization.
     *
     * Called when the application language changes.
     */
    void retranslateUi();

public:
    /**
     * @brief Constructs a new VisualDeckDisplayOptionsWidget.
     * @param parent The parent QWidget—may trigger cloning of models if the parent is a visual deck editor.
     */
    explicit VisualDeckDisplayOptionsWidget(QWidget *parent);

    /**
     * @brief Gets the current display type (Overlap or Flat).
     */
    DisplayType getDisplayType() const
    {
        return currentDisplayType;
    }

    /**
     * @brief Gets the currently active group-by criterion.
     */
    QString getActiveGroupCriteria() const
    {
        return activeGroupCriteria;
    }

    /**
     * @brief Gets the currently active ordered sort criteria.
     */
    QStringList getActiveSortCriteria() const
    {
        return activeSortCriteria;
    }

private slots:
    /**
     * @brief Slot triggered whenever the sort list is reordered.
     *
     * Reads the QListWidget’s order and emits `sortCriteriaChanged()`.
     */
    void onSortCriteriaChange();

    /**
     * @brief Toggles the display layout between flat and overlapping modes.
     *
     * Emits `displayTypeChanged()`.
     */
    void updateDisplayType();

private:
    /// Layout for grouping and sorting UI elements.
    QHBoxLayout *groupAndSortLayout;

    /// Current deck display type.
    DisplayType currentDisplayType = DisplayType::Overlap;

    /// Button used to toggle the display layout.
    QPushButton *displayTypeButton;

    /// Label for the group-by selector.
    QLabel *groupByLabel;

    /// Combo box listing group-by criteria.
    QComboBox *groupByComboBox;

    /// Currently active group-by criterion.
    QString activeGroupCriteria = "maintype";

    /// Encapsulates the sort settings widgets (label + list).
    SettingsButtonWidget *sortCriteriaButton;

    /// Label for “Sort by”.
    QLabel *sortByLabel;

    /// Descriptive label inside the sort criteria button.
    QLabel *sortLabel;

    /// Draggable list of sort criteria.
    QListWidget *sortByListWidget;

    /// Ordered list of current sort criteria.
    QStringList activeSortCriteria = {"name", "cmc", "colors", "maintype"};
};

#endif // COCKATRICE_VISUAL_DECK_DISPLAY_OPTIONS_WIDGET_H
