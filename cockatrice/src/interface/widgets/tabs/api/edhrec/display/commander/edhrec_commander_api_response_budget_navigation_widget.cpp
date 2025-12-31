#include "edhrec_commander_api_response_budget_navigation_widget.h"

#include <QSet>

EdhrecCommanderApiResponseBudgetNavigationWidget::EdhrecCommanderApiResponseBudgetNavigationWidget(
    QWidget *parent,
    const QString &baseUrl)
    : QWidget(parent)
{
    layout = new QGridLayout(this);
    setLayout(layout);

    budgetLabel = new QLabel(this);

    layout->addWidget(budgetLabel, 3, 0, 1, 2);

    for (int i = 0; i < budgetOptions.length(); i++) {
        QString option = budgetOptions.at(i);
        QString label = option.isEmpty() ? "Any" : option.at(0).toUpper() + option.mid(1);
        QPushButton *btn = new QPushButton(label, this);
        btn->setMinimumHeight(84);
        btn->setStyleSheet("font-size: 24px");
        budgetButtons[option] = btn;
        layout->addWidget(btn, 4, i);
        connect(btn, &QPushButton::clicked, this, [=, this]() {
            selectedBudget = option;
            updateOptionButtonSelection(budgetButtons, option);
            emit requestNavigation();
        });
    }

    updateOptionButtonSelection(budgetButtons, "");

    retranslateUi();
    applyOptionsFromUrl(baseUrl);
}

void EdhrecCommanderApiResponseBudgetNavigationWidget::retranslateUi()
{
    budgetLabel->setText(tr("Budget"));
}

void EdhrecCommanderApiResponseBudgetNavigationWidget::applyOptionsFromUrl(const QString &url)
{
    QString cleanedUrl = url;

    // Remove base and file extension
    if (cleanedUrl.startsWith("https://json.edhrec.com/pages/")) {
        cleanedUrl = cleanedUrl.mid(QString("https://json.edhrec.com/pages/").length());
    }
    if (cleanedUrl.endsWith(".json")) {
        cleanedUrl.chop(5);
    }

    // Expecting something like: "commanders/the-ur-dragon/core/expensive"
    QStringList parts = cleanedUrl.split('/', Qt::SkipEmptyParts);

    if (parts.size() < 2) {
        return;
    }

    QString commanderName = parts[1];
    QString gameChangerOpt, budgetOpt;

    // Define valid sets
    QSet<QString> validGameChangers = {"exhibition", "core", "upgraded", "optimized", "cedh"};
    QSet<QString> validBudgets = {"budget", "expensive"};

    // Check remaining parts after commander
    for (int i = 2; i < parts.size(); ++i) {
        QString part = parts[i].toLower();
        if (validGameChangers.contains(part)) {
            gameChangerOpt = part;
        } else if (validBudgets.contains(part)) {
            budgetOpt = part;
        }
    }

    if (!budgetButtons.contains(budgetOpt)) {
        budgetOpt.clear();
    }

    selectedBudget = budgetOpt;

    updateOptionButtonSelection(budgetButtons, selectedBudget);
}

void EdhrecCommanderApiResponseBudgetNavigationWidget::updateOptionButtonSelection(
    QMap<QString, QPushButton *> &buttons,
    const QString &selectedKey)
{
    for (auto it = buttons.begin(); it != buttons.end(); ++it) {
        it.value()->setStyleSheet(it.key() == selectedKey
                                      ? "background-color: lightblue; font-weight: bold; font-size: 24px;"
                                      : "font-size: 24px");
    }
}