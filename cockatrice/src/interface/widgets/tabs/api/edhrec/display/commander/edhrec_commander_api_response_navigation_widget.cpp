#include "edhrec_commander_api_response_navigation_widget.h"

#include "../../tab_edhrec_main.h"

EdhrecCommanderApiResponseNavigationWidget::EdhrecCommanderApiResponseNavigationWidget(
    QWidget *parent,
    const EdhrecCommanderApiResponseCommanderDetails &_commanderDetails,
    QString baseUrl)
    : QWidget(parent), commanderDetails(_commanderDetails)
{
    layout = new QGridLayout(this);
    setLayout(layout);

    bracketNavigationWidget = new EdhrecCommanderApiResponseBracketNavigationWidget(this, baseUrl);

    connect(bracketNavigationWidget, &EdhrecCommanderApiResponseBracketNavigationWidget::requestNavigation, this,
            &EdhrecCommanderApiResponseNavigationWidget::actRequestCommanderNavigation);

    budgetNavigationWidget = new EdhrecCommanderApiResponseBudgetNavigationWidget(this, baseUrl);

    connect(budgetNavigationWidget, &EdhrecCommanderApiResponseBudgetNavigationWidget::requestNavigation, this,
            &EdhrecCommanderApiResponseNavigationWidget::actRequestCommanderNavigation);

    comboPushButton = new QPushButton(this);
    comboPushButton->setMinimumHeight(84);
    comboPushButton->setStyleSheet("font-size: 24px");
    averageDeckPushButton = new QPushButton(this);
    averageDeckPushButton->setMinimumHeight(84);
    averageDeckPushButton->setStyleSheet("font-size: 24px");

    layout->addWidget(comboPushButton, 0, 0, 1, 1);
    layout->addWidget(averageDeckPushButton, 0, 1, 1, 1);

    layout->addWidget(bracketNavigationWidget, 1, 0, 1, 2);
    layout->addWidget(budgetNavigationWidget, 2, 0, 1, 2);

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
}

void EdhrecCommanderApiResponseNavigationWidget::retranslateUi()
{
    comboPushButton->setText(tr("Combos"));
    averageDeckPushButton->setText(tr("Average Deck"));
}

QString EdhrecCommanderApiResponseNavigationWidget::addNavigationOptionsToUrl(QString baseUrl)
{
    if (!bracketNavigationWidget->getSelectedGameChanger().isEmpty()) {
        baseUrl += "/" + bracketNavigationWidget->getSelectedGameChanger();
    }
    if (!budgetNavigationWidget->getSelectedBudget().isEmpty()) {
        baseUrl += "/" + budgetNavigationWidget->getSelectedBudget();
    }
    return baseUrl;
}

void EdhrecCommanderApiResponseNavigationWidget::actRequestCommanderNavigation()
{
    emit requestUrl(addNavigationOptionsToUrl("/commanders/" + commanderDetails.getSanitized()));
}

void EdhrecCommanderApiResponseNavigationWidget::actRequestComboNavigation()
{
    emit requestUrl("/combos/" + commanderDetails.getSanitized());
}

void EdhrecCommanderApiResponseNavigationWidget::actRequestAverageDeckNavigation()
{
    emit requestUrl(addNavigationOptionsToUrl("/average-decks/" + commanderDetails.getSanitized()));
}