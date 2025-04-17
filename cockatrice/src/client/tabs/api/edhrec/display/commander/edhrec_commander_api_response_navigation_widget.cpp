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

    gameChangerLabel = new QLabel(this);
    budgetLabel = new QLabel(this);

    comboPushButton = new QPushButton(this);
    averageDeckPushButton = new QPushButton(this);

    layout->addWidget(comboPushButton, 0, 0, 1, 1);
    layout->addWidget(averageDeckPushButton, 0, 1, 1, 1);

    layout->addWidget(gameChangerLabel, 1, 0, 1, 2);

    for (int i = 0; i < gameChangerOptions.length(); i++) {
        QString option = gameChangerOptions.at(i);
        QString label = option.isEmpty() ? "All" : option.at(0).toUpper() + option.mid(1);
        QPushButton *optionButton = new QPushButton(label, this);
        gameChangerButtons[option] = optionButton;
        layout->addWidget(optionButton, 2, i);
        connect(optionButton, &QPushButton::clicked, this, [=, this]() {
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
    applyOptionsFromUrl(baseUrl);
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
    QString cleanedUrl = url;

    // Remove base and file extension
    if (cleanedUrl.startsWith("https://json.edhrec.com/pages/")) {
        cleanedUrl = cleanedUrl.mid(QString("https://json.edhrec.com/pages/").length());
    }
    if (cleanedUrl.endsWith(".json")) {
        cleanedUrl.chop(5);
    }

    // Expecting something like: "commanders/the-ur-dragon/core/expensive"
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QStringList parts = cleanedUrl.split('/', Qt::SkipEmptyParts);
#else
    QStringList parts = cleanedUrl.split('/', QString::SkipEmptyParts);
#endif

    if (parts.size() < 2) {
        return;
    }

    QString commanderName = parts[1];
    QString gameChangerOpt, budgetOpt;

    // Define valid sets
    QSet<QString> validGameChangers = {"core", "upgraded", "optimized"};
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

    // Validate and apply
    if (!gameChangerButtons.contains(gameChangerOpt)) {
        gameChangerOpt.clear();
    }
    if (!budgetButtons.contains(budgetOpt)) {
        budgetOpt.clear();
    }

    selectedGameChanger = gameChangerOpt;
    selectedBudget = budgetOpt;

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

QString EdhrecCommanderApiResponseNavigationWidget::addNavigationOptionsToUrl(QString baseUrl)
{
    if (!selectedGameChanger.isEmpty()) {
        baseUrl += "/" + selectedGameChanger;
    }
    if (!selectedBudget.isEmpty()) {
        baseUrl += "/" + selectedBudget;
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