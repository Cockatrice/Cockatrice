#ifndef COCKATRICE_EDHREC_COMMANDER_API_RESPONSE_BUDGET_NAVIGATION_WIDGET_H
#define COCKATRICE_EDHREC_COMMANDER_API_RESPONSE_BUDGET_NAVIGATION_WIDGET_H

#include <QGridLayout>
#include <QLabel>
#include <QMap>
#include <QPushButton>
#include <QWidget>

class EdhrecCommanderApiResponseBudgetNavigationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EdhrecCommanderApiResponseBudgetNavigationWidget(QWidget *parent, const QString &baseUrl);
    void retranslateUi();
    void applyOptionsFromUrl(const QString &url);
    QString getSelectedBudget() const
    {
        return selectedBudget;
    }

signals:
    void requestNavigation();

private:
    QGridLayout *layout;
    QLabel *budgetLabel;

    QStringList budgetOptions = {"", "budget", "expensive"};
    QString selectedBudget;

    QMap<QString, QPushButton *> budgetButtons;

    void updateOptionButtonSelection(QMap<QString, QPushButton *> &buttons, const QString &selectedKey);
};

#endif // COCKATRICE_EDHREC_COMMANDER_API_RESPONSE_BUDGET_NAVIGATION_WIDGET_H
