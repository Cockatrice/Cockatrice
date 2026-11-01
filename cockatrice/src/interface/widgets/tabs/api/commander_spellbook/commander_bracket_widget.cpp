#include "commander_bracket_widget.h"

#include "../../../../../client/settings/cache_settings.h"
#include "../../../../pixel_map_generator.h"
#include "commander_bracket_service.h"
#include "libcockatrice/deck_list/deck_list.h"

#include <QAbstractButton>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QObject>
#include <QPushButton>
#include <QStringLiteral>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <libcockatrice/settings/deck_editor_settings.h>
#include <libcockatrice/utility/qt_utils.h>
#include <qnamespace.h>

template <class T> class QSharedPointer;

CommanderBracketWidget::CommanderBracketWidget(QWidget *parent) : QWidget(parent)
{
    autoEstimateTimer = new QTimer(this);
    autoEstimateTimer->setSingleShot(true);
    connect(autoEstimateTimer, &QTimer::timeout, this, [this]() {
        if (SettingsCache::instance().deckEditor().getCommanderSpellbookIntegrationEnabled() ==
            commanderSpellbookIntegrationEnabledIndexAutomatic) {
            requestBracketEstimate();
        }
    });
    bracketLabel = new QLabel(tr("Bracket:"), this);

    bracketValueLabel = new QLabel(this);
    bracketValueLabel->setTextFormat(Qt::PlainText);
    bracketValueLabel->setText("-");
    bracketValueLabel->setObjectName("bracketValueLabel");

    bracketInfoButton = new QToolButton(this);
    bracketInfoButton->setText("?");
    bracketInfoButton->setAutoRaise(true);
    bracketInfoButton->setEnabled(false);

    bracketRefreshButton = new QToolButton(this);
    bracketRefreshButton->setIcon(themePixmap(QStringLiteral("icons/reload")));
    bracketRefreshButton->setAutoRaise(true);

    connect(bracketRefreshButton, &QToolButton::clicked, this, &CommanderBracketWidget::requestBracketEstimate);

    auto *layout = QtUtils::makeLayout<QGridLayout>(this);
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
            &CommanderBracketWidget::displayEstimate);

    setVisible(false);
}

CommanderBracketWidget::~CommanderBracketWidget()
{
    cancelEstimate();
}

void CommanderBracketWidget::cancelEstimate()
{
    autoEstimateTimer->stop();
    const quint64 canceledRequest = requestId;
    requestId = 0;
    CommanderBracketService::instance().cancelEstimate(canceledRequest);
    bracketRefreshButton->setEnabled(true);
    if (canceledRequest) {
        bracketValueLabel->setText("-");
        bracketValueLabel->setToolTip({});
    }
}

void CommanderBracketWidget::setDeck(const QSharedPointer<DeckList> &_deck)
{
    cancelEstimate();
    deck = _deck;
    hasEstimate = false;

    // Reset the displayed bracket
    bracketValueLabel->setText("-");
    bracketValueLabel->setToolTip({});
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
    auto *labelLayout = QtUtils::makeLayout<QHBoxLayout>(labelWidget);

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
    if (clickedButton == enableBtn || clickedButton == automaticBtn) {
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
    const int mode = SettingsCache::instance().deckEditor().getCommanderSpellbookIntegrationEnabled();
    if (!deck || deck->getGameFormat().compare("commander", Qt::CaseInsensitive) != 0 || requestId ||
        (mode != commanderSpellbookIntegrationEnabledIndexEnabled &&
         mode != commanderSpellbookIntegrationEnabledIndexAutomatic)) {
        return;
    }

    autoEstimateTimer->stop();
    hasEstimate = false;
    bracketRefreshButton->setEnabled(false);
    bracketInfoButton->setEnabled(false);
    bracketInfoButton->setToolTip({});
    bracketValueLabel->setText(tr("Calculating…"));
    bracketValueLabel->setToolTip({});

    requestId = CommanderBracketService::instance().estimateBracket(*deck, this);
}

void CommanderBracketWidget::onEstimateBracketFinished(quint64 id,
                                                       QObject *requester,
                                                       const CommanderBracketEstimate &result)
{
    if (requester != this || id != requestId) {
        return;
    }

    requestId = 0;
    hasEstimate = true;
    lastEstimate = result;
    displayEstimate();
    bracketRefreshButton->setEnabled(true);
}

void CommanderBracketWidget::displayEstimate()
{
    if (!hasEstimate) {
        return;
    }

    BracketExplainer explainer;
    lastBracketExplanation = explainer.explain(lastEstimate.rawResult);

    // Display bracket
    bracketValueLabel->setText(
        SettingsCache::instance().deckEditor().getCommanderSpellbookIntegrationUseOfficialBracketNames()
            ? lastEstimate.officialName
            : lastEstimate.displayName);

    // Build tooltip
    QString tooltip;
    for (const auto &section : lastBracketExplanation.sections) {
        tooltip += "<b>" + section.title.toHtmlEscaped() + "</b><br>";
        for (const auto &line : section.bulletPoints) {
            tooltip += "• " + line.toHtmlEscaped() + "<br>";
        }
        tooltip += "<br>";
    }

    bracketInfoButton->setToolTip(tooltip);
    bracketInfoButton->setEnabled(!tooltip.isEmpty());
}

void CommanderBracketWidget::onEstimateBracketError(quint64 id, QObject *requester, const QString &error)
{
    if (requester != this || id != requestId) {
        return;
    }

    requestId = 0;
    hasEstimate = false;
    bracketValueLabel->setText("-");
    bracketValueLabel->setToolTip(
        QStringLiteral("<qt>%1</qt>").arg(tr("Unable to estimate bracket: %1").arg(error).toHtmlEscaped()));
    bracketRefreshButton->setEnabled(true);
    bracketInfoButton->setToolTip({});
    bracketInfoButton->setEnabled(false);
}

void CommanderBracketWidget::maybeAutoEstimateBracket()
{
    if (prompting) {
        return;
    }
    autoEstimateTimer->stop();
    if (!deck) {
        cancelEstimate();
        updateBracketVisibility(false);
        return;
    }
    const QString formatKey = deck->getGameFormat();

    const bool isCommander = (formatKey.compare("commander", Qt::CaseInsensitive) == 0);

    int mode = SettingsCache::instance().deckEditor().getCommanderSpellbookIntegrationEnabled();

    if (!isCommander || mode == commanderSpellbookIntegrationEnabledIndexDisabled) {
        cancelEstimate();
        updateBracketVisibility(false);
        return;
    }

    if (mode == commanderSpellbookIntegrationEnabledIndexUnprompted) {
        const auto promptedDeck = deck;
        prompting = true;
        const bool accepted = promptCommanderSpellbookIntegration();
        prompting = false;
        if (deck != promptedDeck || !deck || deck->getGameFormat().compare("commander", Qt::CaseInsensitive) != 0) {
            maybeAutoEstimateBracket();
            return;
        }
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
    if (requestId || hasEstimate) {
        return;
    }

    // Defer to avoid races during init / model rebuild
    autoEstimateTimer->start(0);
}

void CommanderBracketWidget::retranslateUi()
{
    bracketLabel->setText(tr("Bracket:"));
    displayEstimate();
    bracketRefreshButton->setToolTip(tr("Recalculate bracket"));
}
