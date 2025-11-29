#ifndef COCKATRICE_VISUAL_DECK_DISPLAY_OPTIONS_WIDGET_H
#define COCKATRICE_VISUAL_DECK_DISPLAY_OPTIONS_WIDGET_H
#include "visual_deck_editor_widget.h"

#include <QPushButton>
#include <QWidget>

class VisualDeckDisplayOptionsWidget : public QWidget
{
    Q_OBJECT
signals:
    void displayTypeChanged(const DisplayType &displayType);
    void groupCriteriaChanged(const QString &activeGroupCriteria);
    void sortCriteriaChanged(const QStringList &activeSortCriteria);

public slots:
    void retranslateUi();

public:
    explicit VisualDeckDisplayOptionsWidget(QWidget *parent);

    DisplayType getDisplayType() const
    {
        return currentDisplayType;
    }

    QString getActiveGroupCriteria() const
    {
        return activeGroupCriteria;
    }

    QStringList getActiveSortCriteria() const
    {
        return activeSortCriteria;
    }

private slots:
    void onSortCriteriaChange();
    void updateDisplayType();

private:
    QHBoxLayout *groupAndSortLayout;
    DisplayType currentDisplayType = DisplayType::Overlap;
    QPushButton *displayTypeButton;
    QLabel *groupByLabel;
    QComboBox *groupByComboBox;
    QString activeGroupCriteria = "maintype";
    SettingsButtonWidget *sortCriteriaButton;
    QLabel *sortByLabel;
    QLabel *sortLabel;
    QListWidget *sortByListWidget;
    QStringList activeSortCriteria = {"name", "cmc", "colors", "maintype"};
};

#endif // COCKATRICE_VISUAL_DECK_DISPLAY_OPTIONS_WIDGET_H
