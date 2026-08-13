#include "commander_bracket_widget.h"

#include "../../../../../client/settings/cache_settings.h"
#include "commander_bracket_service.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <libcockatrice/settings/deck_editor_settings.h>

CommanderBracketWidget::CommanderBracketWidget(QWidget *parent) : QWidget(parent)
{
    bracketLabel = new QLabel(tr("Bracket:"), this);

    bracketValueLabel = new QLabel(this);
    bracketValueLabel->setText("-");
    bracketValueLabel->setObjectName("bracketValueLabel");

    bracketInfoButton = new QToolButton(this);
    bracketInfoButton->setText("?");
    bracketInfoButton->setAutoRaise(true);
    bracketInfoButton->setEnabled(false);

    bracketRefreshButton = new QToolButton(this);
    bracketRefreshButton->setIcon(QPixmap("theme:icons/reload"));
    bracketRefreshButton->setAutoRaise(true);

    connect(bracketRefreshButton, &QToolButton::clicked, this, &CommanderBracketWidget::requestBracketEstimate);

    auto *layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(bracketLabel, 0, 0);

    auto *bracketRow = new QHBoxLayout;
    bracketRow->addWidget(bracketValueLabel);
    bracketRow->addWidget(bracketInfoButton);
    bracketRow->addWidget(bracketRefreshButton);
    bracketRow->addStretch();

    layout->addLayout(bracketRow, 0, 1);

    connect(&CommanderBracketService::instance(), &CommanderBracketService::estimateFinished, this,
            &CommanderBracketWidget::onEstimateBracketFinished);
    connect(&CommanderBracketService::instance(), &CommanderBracketService::estimateError, this,
            &CommanderBracketWidget::onEstimateBracketError);

    connect(&SettingsCache::instance().deckEditor(), &DeckEditorSettings::commanderSpellbookIntegrationEnabledChanged,
            this, &CommanderBracketWidget::maybeAutoEstimateBracket);
    connect(&SettingsCache::instance().deckEditor(),
            &DeckEditorSettings::commanderSpellbookIntegrationUseOfficialBracketNamesChanged, this,
            &CommanderBracketWidget::maybeAutoEstimateBracket);

    setVisible(false);
}

void CommanderBracketWidget::setDeck(const QSharedPointer<DeckList> &_deck)
{
    deck = _deck;
    requestId = 0; // invalidate any in-flight estimate for the previous deck

    // Reset the displayed bracket
    bracketValueLabel->setText("-");
    bracketInfoButton->setToolTip({});
    bracketInfoButton->setEnabled(false);
    bracketRefreshButton->setEnabled(true);

    maybeAutoEstimateBracket();
}

bool CommanderBracketWidget::promptCommanderSpellbookIntegration()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("CommanderSpellbook integration"));

    auto *mainLayout = new QVBoxLayout(&dialog);

    // Main text
    auto *label = new QLabel(tr("CommanderSpellbook can analyze your deck and estimate its Commander bracket.\n\n"
                                "This sends your deck list to an external service.\n\n"
                                "CommanderSpellbook uses its own bracket naming system based on their own algorithm. "
                                "These names can be mapped to the official Commander brackets, but the mapping "
                                "is only an approximation."));
    label->setWordWrap(true);
    mainLayout->addWidget(label);

    // Naming selector
    auto *formLayout = new QFormLayout;
    auto *namingCombo = new QComboBox(&dialog);
    namingCombo->addItem(tr("CommanderSpellbook bracket names"));
    namingCombo->addItem(tr("Official Commander bracket names (approximate)"));
    namingCombo->setCurrentIndex(
        SettingsCache::instance().deckEditor().getCommanderSpellbookIntegrationUseOfficialBracketNames() ? 1 : 0);

    // Create label + explainer button
    auto *labelWidget = new QWidget(&dialog);
    auto *labelLayout = new QHBoxLayout(labelWidget);
    labelLayout->setContentsMargins(0, 0, 0, 0);

    auto *namingLabel = new QLabel(tr("Bracket naming:"), labelWidget);
    auto *explainerButton = new QToolButton(labelWidget);
    explainerButton->setText("?");
    explainerButton->setAutoRaise(true);
    explainerButton->setEnabled(false);
    explainerButton->setToolTip(CommanderBracketNames::Explainer);

    labelLayout->addWidget(namingLabel);
    labelLayout->addWidget(explainerButton);
    labelLayout->addStretch(); // push the button next to label, combo stays aligned

    // Add row with the custom label widget
    formLayout->addRow(labelWidget, namingCombo);
    mainLayout->addLayout(formLayout);

    // Buttons
    auto *buttonBox = new QDialogButtonBox(&dialog);
    auto *enableBtn = buttonBox->addButton(tr("Enable"), QDialogButtonBox::AcceptRole);
    auto *automaticBtn = buttonBox->addButton(tr("Automatic"), QDialogButtonBox::ApplyRole);
    auto *disableBtn = buttonBox->addButton(tr("Disable"), QDialogButtonBox::RejectRole);
    mainLayout->addWidget(buttonBox);

    // Track which button was clicked
    QAbstractButton *clickedButton = nullptr;
    QObject::connect(buttonBox, &QDialogButtonBox::clicked, &dialog, [&](QAbstractButton *btn) {
        clickedButton = btn;
        dialog.accept();
    });

    dialog.exec();

    // Persist naming choice (if not disabled)
    if (clickedButton != disableBtn) {
        bool useOfficial = namingCombo->currentIndex() == 1;
        SettingsCache::instance().deckEditor().setCommanderSpellbookIntegrationUseOfficialBracketNames(useOfficial);
    }

    // Persist integration mode
    if (clickedButton == disableBtn) {
        SettingsCache::instance().deckEditor().setCommanderSpellbookIntegrationEnabled(
            commanderSpellbookIntegrationEnabledIndexDisabled);
        return false;
    }
    if (clickedButton == enableBtn) {
        SettingsCache::instance().deckEditor().setCommanderSpellbookIntegrationEnabled(
            commanderSpellbookIntegrationEnabledIndexEnabled);
        return true;
    }
    if (clickedButton == automaticBtn) {
        SettingsCache::instance().deckEditor().setCommanderSpellbookIntegrationEnabled(
            commanderSpellbookIntegrationEnabledIndexAutomatic);
        return true;
    }

    return false;
}

void CommanderBracketWidget::updateBracketVisibility(bool visible)
{
    setVisible(visible);
}

void CommanderBracketWidget::requestBracketEstimate()
{
    bracketRefreshButton->setEnabled(false);
    bracketInfoButton->setEnabled(false);
    bracketValueLabel->setText(tr("Calculating…"));

    requestId = CommanderBracketService::instance().estimateBracket(*deck, this);
}

void CommanderBracketWidget::onEstimateBracketFinished(quint64 id,
                                                       QObject *requester,
                                                       const CommanderBracketEstimate &result)
{
    if (requester != this || id != requestId) {
        return;
    }

    BracketExplainer explainer;
    lastBracketExplanation = explainer.explain(result.rawResult);

    // Display bracket
    bracketValueLabel->setText(
        SettingsCache::instance().deckEditor().getCommanderSpellbookIntegrationUseOfficialBracketNames()
            ? result.officialName
            : result.displayName);
    bracketRefreshButton->setEnabled(true);

    // Build tooltip
    QString tooltip;
    for (const auto &section : lastBracketExplanation.sections) {
        tooltip += "<b>" + section.title + "</b><br>";
        for (const auto &line : section.bulletPoints) {
            tooltip += "• " + line + "<br>";
        }
        tooltip += "<br>";
    }

    bracketInfoButton->setToolTip(tooltip);
    bracketInfoButton->setEnabled(!tooltip.isEmpty());
}

void CommanderBracketWidget::onEstimateBracketError(quint64 id, QObject *requester, const QString & /*error*/)
{
    if (requester != this || id != requestId) {
        return;
    }

    bracketValueLabel->setText("-");
    bracketRefreshButton->setEnabled(true);
    bracketInfoButton->setToolTip({});
    bracketInfoButton->setEnabled(false);
}

void CommanderBracketWidget::maybeAutoEstimateBracket()
{
    const QString formatKey = deck->getGameFormat();

    const bool isCommander = (formatKey.compare("commander", Qt::CaseInsensitive) == 0);

    int mode = SettingsCache::instance().deckEditor().getCommanderSpellbookIntegrationEnabled();

    if (!isCommander || mode == commanderSpellbookIntegrationEnabledIndexDisabled) {
        updateBracketVisibility(false);
        return;
    }

    if (mode == commanderSpellbookIntegrationEnabledIndexUnprompted) {
        if (prompting) {
            return;
        }
        prompting = true;
        const bool accepted = promptCommanderSpellbookIntegration();
        prompting = false;
        if (!accepted) {
            updateBracketVisibility(false);
            return;
        }
    }

    updateBracketVisibility(true);

    mode = SettingsCache::instance().deckEditor().getCommanderSpellbookIntegrationEnabled();
    if (mode != commanderSpellbookIntegrationEnabledIndexAutomatic) {
        return;
    }

    // Avoid firing if we already have a result or a request in flight
    if (!bracketRefreshButton->isEnabled()) {
        return;
    }

    // Defer to avoid races during init / model rebuild
    QTimer::singleShot(0, this, &CommanderBracketWidget::requestBracketEstimate);
}

void CommanderBracketWidget::retranslateUi()
{
    bracketLabel->setText(tr("Bracket:"));
    bracketInfoButton->setToolTip(tr("Why this bracket?"));
    bracketRefreshButton->setToolTip(tr("Recalculate bracket"));
}
