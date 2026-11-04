#ifndef COCKATRICE_VISUAL_DATABASE_DISPLAY_GENDER_FILTER_WIDGET_H
#define COCKATRICE_VISUAL_DATABASE_DISPLAY_GENDER_FILTER_WIDGET_H

#include <QMap>
#include <QString>
#include <QWidget>

class FilterTreeModel;
class FlowWidget;
class QPushButton;

// Uses the literal MSE gender option values, not labels or filename suffixes.
class VisualDatabaseDisplayGenderFilterWidget : public QWidget
{
public:
    explicit VisualDatabaseDisplayGenderFilterWidget(QWidget *parent, FilterTreeModel *filterModel);

private:
    FilterTreeModel *filterModel;
    QMap<QString, QPushButton *> genderButtons;

    void updateGenderFilter();
    void syncWithFilterModel();
};

#endif // COCKATRICE_VISUAL_DATABASE_DISPLAY_GENDER_FILTER_WIDGET_H
