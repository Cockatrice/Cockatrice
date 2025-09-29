/**
 * @file visual_database_display_color_filter_widget.h
 * @ingroup VisualCardDatabaseWidgets
 * @brief TODO: Document this.
 */

#ifndef VISUAL_DATABASE_DISPLAY_COLOR_FILTER_WIDGET_H
#define VISUAL_DATABASE_DISPLAY_COLOR_FILTER_WIDGET_H

#include "../../../filters/filter_tree_model.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>

class VisualDatabaseDisplayColorFilterCircleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualDatabaseDisplayColorFilterCircleWidget(QChar color, QWidget *parent = nullptr);
    void setColorActive(bool active);
    bool isColorActive() const;
    QChar getColorChar() const;

signals:
    void colorToggled(QChar color, bool active);

private:
    QChar colorChar;
    bool isActive;
    int circleDiameter;
};

enum class FilterMode
{
    ExactMatch,    // Only selected colors are included, all others are excluded.
    Includes,      // Include selected colors (OR condition).
    IncludeExclude // Include selected colors (OR) and exclude unselected colors (AND NOT).
};

class VisualDatabaseDisplayColorFilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualDatabaseDisplayColorFilterWidget(QWidget *parent, FilterTreeModel *filterModel);
    void retranslateUi();

signals:
    void filterModeChanged(FilterMode filterMode);

private slots:
    void handleColorToggled(QChar color, bool active);
    void addFilter(QChar color);
    void removeFilter(QChar color);
    void updateFilterMode();
    void setManaSymbolActive(QChar color, bool active);
    QList<QChar> getActiveColors();
    void syncWithFilterModel();

private:
    FilterTreeModel *filterModel;
    QHBoxLayout *layout;
    QPushButton *toggleButton;
    FilterMode currentMode = FilterMode::Includes; // Default mode
};

#endif // VISUAL_DATABASE_DISPLAY_COLOR_FILTER_WIDGET_H
