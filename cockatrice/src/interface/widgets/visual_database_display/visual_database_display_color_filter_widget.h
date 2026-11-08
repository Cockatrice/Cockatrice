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
#include <QWidget>
#include <libcockatrice/filters/filter_mode.h>
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
