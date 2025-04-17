#include "edhrec_commander_api_response_navigation_widget.h"

#include "../../tab_edhrec_main.h"

EdhrecCommanderApiResponseNavigationWidget::EdhrecCommanderApiResponseNavigationWidget(
    QWidget *parent,
    const EdhrecCommanderApiResponseCommanderDetails &_commanderDetails,
    QUrl baseUrl)
    : QWidget(parent), commanderDetails(_commanderDetails)
{
    qInfo() << commanderDetails.getUrl();
    layout = new QGridLayout(this);
    setLayout(layout);

    gameChangerLabel = new QLabel(this);
    budgetLabel = new QLabel(this);

    commanderDetails.debugPrint();

    comboPushButton = new QPushButton(this);
    averageDeckPushButton = new QPushButton(this);

    layout->addWidget(comboPushButton, 0, 0, 1, 1);
    layout->addWidget(averageDeckPushButton, 0, 1, 1, 1);

    layout->addWidget(gameChangerLabel, 1, 0, 1, 2);

    for (int i = 0; i < gameChangerOptions.length(); i++) {
        QString option = gameChangerOptions.at(i);
        QString label = option.isEmpty() ? "Any" : option.at(0).toUpper() + option.mid(1);
        QPushButton *btn = new QPushButton(label, this);
        gameChangerButtons[option] = btn;
        layout->addWidget(btn, 2, i);
        connect(btn, &QPushButton::clicked, this, [=, this]() {
            selectedGameChanger = option;
            updateOptionButtonSelection(gameChangerButtons, option);
            actRequestCommanderNavigation();
        });
    }

    layout->addWidget(budgetLabel, 3, 0, 1, 2);

    for (int i = 0; i < budgetOptions.length(); i++) {
        QString option = budgetOptions.at(i);
        QString label = option.isEmpty() ? "Any" : option.at(0).toUpper() + option.mid(1);
        QPushButton *btn = new QPushButton(label, this);
        budgetButtons[option] = btn;
        layout->addWidget(btn, 4, i);
        connect(btn, &QPushButton::clicked, this, [=, this]() {
            selectedBudget = option;
            updateOptionButtonSelection(budgetButtons, option);
            actRequestCommanderNavigation();
        });
    }

    updateOptionButtonSelection(gameChangerButtons, "");
    updateOptionButtonSelection(budgetButtons, "");

    QWidget *currentParent = parentWidget();
    TabEdhRecMain *parentTab = nullptr;

    while (currentParent) {
        if ((parentTab = qobject_cast<TabEdhRecMain *>(currentParent))) {
            break;
        }
        currentParent = currentParent->parentWidget();
    }

    if (parentTab) {
        connect(comboPushButton, &QPushButton::clicked, this,
                &EdhrecCommanderApiResponseNavigationWidget::actRequestComboNavigation);
        connect(averageDeckPushButton, &QPushButton::clicked, this,
                &EdhrecCommanderApiResponseNavigationWidget::actRequestAverageDeckNavigation);
        connect(this, &EdhrecCommanderApiResponseNavigationWidget::requestUrl, parentTab,
                &TabEdhRecMain::actNavigatePage);
    }

    retranslateUi();
    applyOptionsFromUrl(baseUrl.toString());
}

void EdhrecCommanderApiResponseNavigationWidget::retranslateUi()
{
    comboPushButton->setText(tr("Combos"));
    averageDeckPushButton->setText(tr("Average Deck"));
    gameChangerLabel->setText(tr("Game Changers"));
    budgetLabel->setText(tr("Budget"));
}

void EdhrecCommanderApiResponseNavigationWidget::applyOptionsFromUrl(const QString &url)
{
    // Example input: /commanders/kaalia-of-the-vast/core/budget
    QStringList parts = url.split('/', Qt::SkipEmptyParts);

    // Make sure there's at least a base + commander name
    if (parts.size() < 2)
        return;

    // Extract extra segments, if any
    QString gameChangerOpt, budgetOpt;

    if (parts.size() >= 3)
        gameChangerOpt = parts[2]; // this would be "core", "upgraded", etc.
    if (parts.size() >= 4)
        budgetOpt = parts[3]; // "budget" or "expensive"

    // Defensive: only accept known values
    if (!gameChangerButtons.contains(gameChangerOpt))
        gameChangerOpt = "";
    if (!budgetButtons.contains(budgetOpt))
        budgetOpt = "";

    // Apply internal state
    selectedGameChanger = gameChangerOpt;
    selectedBudget = budgetOpt;

    // Apply visual state
    updateOptionButtonSelection(gameChangerButtons, selectedGameChanger);
    updateOptionButtonSelection(budgetButtons, selectedBudget);
}

void EdhrecCommanderApiResponseNavigationWidget::updateOptionButtonSelection(QMap<QString, QPushButton *> &buttons,
                                                                             const QString &selectedKey)
{
    for (auto it = buttons.begin(); it != buttons.end(); ++it) {
        it.value()->setStyleSheet(it.key() == selectedKey ? "background-color: lightblue; font-weight: bold;" : "");
    }
}

void EdhrecCommanderApiResponseNavigationWidget::actRequestCommanderNavigation()
{
    QString url = "/commanders/" + commanderDetails.getSanitized();
    if (!selectedGameChanger.isEmpty())
        url += "/" + selectedGameChanger;
    if (!selectedBudget.isEmpty())
        url += "/" + selectedBudget;
    emit requestUrl(url);
}

void EdhrecCommanderApiResponseNavigationWidget::actRequestComboNavigation()
{
    QString url = "/combos/" + commanderDetails.getSanitized();
    if (!selectedGameChanger.isEmpty())
        url += "/" + selectedGameChanger;
    if (!selectedBudget.isEmpty())
        url += "/" + selectedBudget;
    emit requestUrl(url);
}

void EdhrecCommanderApiResponseNavigationWidget::actRequestAverageDeckNavigation()
{
    QString url = "/average-decks/" + commanderDetails.getSanitized();
    if (!selectedGameChanger.isEmpty())
        url += "/" + selectedGameChanger;
    if (!selectedBudget.isEmpty())
        url += "/" + selectedBudget;
    emit requestUrl(url);
}