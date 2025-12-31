#include "edhrec_commander_api_response_bracket_navigation_widget.h"

#include <QSet>

EdhrecCommanderApiResponseBracketNavigationWidget::EdhrecCommanderApiResponseBracketNavigationWidget(
    QWidget *parent,
    const QString &baseUrl)
    : QWidget(parent)
{
    layout = new QGridLayout(this);
    setLayout(layout);

    gameChangerLabel = new QLabel(this);

    layout->addWidget(gameChangerLabel, 1, 0, 1, 2);

    for (int i = 0; i < gameChangerOptions.length(); i++) {
        QString option = gameChangerOptions.at(i);
        QString label = option.isEmpty() ? "All" : option.at(0).toUpper() + option.mid(1);
        QPushButton *optionButton = new QPushButton(label, this);
        optionButton->setMinimumHeight(84);
        optionButton->setStyleSheet("font-size: 24px");
        gameChangerButtons[option] = optionButton;
        layout->addWidget(optionButton, 2, i);
        connect(optionButton, &QPushButton::clicked, this, [=, this]() {
            selectedGameChanger = option;
            updateOptionButtonSelection(gameChangerButtons, option);
            emit requestNavigation();
        });
    }

    updateOptionButtonSelection(gameChangerButtons, "");

    retranslateUi();
    applyOptionsFromUrl(baseUrl);
}

void EdhrecCommanderApiResponseBracketNavigationWidget::retranslateUi()
{
    gameChangerLabel->setText(tr("Game Changers"));
}

void EdhrecCommanderApiResponseBracketNavigationWidget::applyOptionsFromUrl(const QString &url)
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
    QString gameChangerOpt;

    // Define valid sets
    QSet<QString> validGameChangers = {"exhibition", "core", "upgraded", "optimized", "cedh"};

    // Check remaining parts after commander
    for (int i = 2; i < parts.size(); ++i) {
        QString part = parts[i].toLower();
        if (validGameChangers.contains(part)) {
            gameChangerOpt = part;
        }
    }

    // Validate and apply
    if (!gameChangerButtons.contains(gameChangerOpt)) {
        gameChangerOpt.clear();
    }

    selectedGameChanger = gameChangerOpt;

    updateOptionButtonSelection(gameChangerButtons, selectedGameChanger);
}

void EdhrecCommanderApiResponseBracketNavigationWidget::updateOptionButtonSelection(
    QMap<QString, QPushButton *> &buttons,
    const QString &selectedKey)
{
    for (auto it = buttons.begin(); it != buttons.end(); ++it) {
        it.value()->setStyleSheet(it.key() == selectedKey
                                      ? "background-color: lightblue; font-weight: bold; font-size: 24px;"
                                      : "font-size: 24px");
    }
}