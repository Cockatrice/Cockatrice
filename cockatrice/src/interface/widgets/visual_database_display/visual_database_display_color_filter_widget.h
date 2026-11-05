/**
 * @file visual_database_display_color_filter_widget.h
 * @ingroup VisualCardDatabaseWidgets
 */
//! \todo Document this file.

#ifndef VISUAL_DATABASE_DISPLAY_COLOR_FILTER_WIDGET_H
#define VISUAL_DATABASE_DISPLAY_COLOR_FILTER_WIDGET_H

#include <QByteArray>
#include <QChar>
#include <QList>
#include <QMetaType>
#include <QWidget>
#include <libcockatrice/utility/debouncer.h>
#include <qtmetamacros.h>

class FilterTreeModel;
class QComboBox;
class QHBoxLayout;

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
    Includes = 0,   // Include selected colors (OR condition).
    Excludes,       // Exclude selected colors (AND NOT).
    IncludeExclude, // Include selected colors (OR) and exclude unselected colors (AND NOT).
    ExactMatch,     // Only selected colors are included, all others are excluded.
    ColorIdentity,  // Keep cards whose color identity is a subset of the selected colors (stored as AND NOT for each
                    // unselected color).
    Custom,         // The color filters do not match any of the above modes. Not included in the combo box.
    End
};

Q_DECLARE_METATYPE(FilterMode)

class VisualDatabaseDisplayColorFilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualDatabaseDisplayColorFilterWidget(QWidget *parent, FilterTreeModel *filterModel);
    void retranslateUi();
    void setFilterMode(FilterMode filterMode);

signals:
    void filterModeChanged(FilterMode filterMode);

private slots:
    void addCustomFilterModeItem();
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
    QComboBox *modeComboBox;
    FilterMode currentMode = FilterMode::Includes; // Default mode
    QtUtils::Debouncer syncDebouncer{100};
};

#endif // VISUAL_DATABASE_DISPLAY_COLOR_FILTER_WIDGET_H
