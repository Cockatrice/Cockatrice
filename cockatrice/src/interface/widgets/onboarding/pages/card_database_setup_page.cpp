#include "card_database_setup_page.h"

#include "../../client/settings/cache_settings.h"

#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QUrl>
#include <QVBoxLayout>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/settings/updates_settings.h>

CardDatabaseSetupPage::CardDatabaseSetupPage(QWidget *parent) : FirstRunWizardPage(parent)
{
    statusLabel = new QLabel(this);
    statusLabel->setWordWrap(true);
    statusLabel->setAlignment(Qt::AlignCenter);

    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 0);
    progressBar->setTextVisible(false);
    progressBar->setFixedWidth(280);

    retryButton = new QPushButton(this);
    manualButton = new QPushButton(this);

    connect(retryButton, &QPushButton::clicked, this, [this] {
        setState(State::Running);
        emit updateRequested();
    });
    connect(manualButton, &QPushButton::clicked, this, &CardDatabaseSetupPage::manualSetupRequested);

    // ── Advanced: custom download source ───────────────────────────────
    advancedToggleButton = new QPushButton(this);
    advancedToggleButton->setCheckable(true);
    advancedToggleButton->setChecked(false);
    advancedToggleButton->setFlat(true);
    advancedToggleButton->setStyleSheet("QPushButton { text-align: left; padding: 5px 12px; font-weight: bold; }"
                                        "QPushButton:checked { }");

    advancedPanel = new QWidget(this);
    advancedPanel->setVisible(false);

    urlLineEdit = new QLineEdit(advancedPanel);
    urlHintLabel = new QLabel(advancedPanel);
    urlHintLabel->setWordWrap(true);

    restoreDefaultUrlButton = new QPushButton(advancedPanel);
    applyAndRetryButton = new QPushButton(advancedPanel);

    connect(advancedToggleButton, &QPushButton::toggled, this, &CardDatabaseSetupPage::onToggleAdvanced);
    connect(restoreDefaultUrlButton, &QPushButton::clicked, this, &CardDatabaseSetupPage::onRestoreDefaultUrl);
    connect(applyAndRetryButton, &QPushButton::clicked, this, &CardDatabaseSetupPage::onApplyCustomUrl);

    auto *advancedButtonRow = new QHBoxLayout;
    advancedButtonRow->addWidget(restoreDefaultUrlButton);
    advancedButtonRow->addStretch();
    advancedButtonRow->addWidget(applyAndRetryButton);

    auto *advancedLayout = new QVBoxLayout(advancedPanel);
    advancedLayout->setContentsMargins(12, 4, 12, 4);
    advancedLayout->addWidget(urlLineEdit);
    advancedLayout->addWidget(urlHintLabel);
    advancedLayout->addLayout(advancedButtonRow);

    // ── Startup card update check ───────────────────────────────────────
    auto &upd = SettingsCache::instance().updates();

    const auto updateBehavior = [this] {
        auto &u = SettingsCache::instance().updates();
        int idx = startupBehaviorCombo->currentIndex();
        u.setStartupCardUpdateCheckPromptForUpdate(idx == 1);
        u.setStartupCardUpdateCheckAlwaysUpdate(idx == 2);
    };

    startupBehaviorLabel = new QLabel(this);
    startupBehaviorCombo = new QComboBox(this);
    startupBehaviorCombo->addItem(QString()); // placeholder, filled in retranslateUi
    startupBehaviorCombo->addItem(QString());
    startupBehaviorCombo->addItem(QString());
    if (upd.getStartupCardUpdateCheckPromptForUpdate()) {
        startupBehaviorCombo->setCurrentIndex(1);
    } else if (upd.getStartupCardUpdateCheckAlwaysUpdate()) {
        startupBehaviorCombo->setCurrentIndex(2);
    } else {
        startupBehaviorCombo->setCurrentIndex(0);
    }
    connect(startupBehaviorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, updateBehavior);

    checkIntervalLabel = new QLabel(this);
    checkIntervalSpinBox = new QSpinBox(this);
    checkIntervalSpinBox->setMinimum(1);
    checkIntervalSpinBox->setMaximum(30);
    checkIntervalSpinBox->setValue(upd.getCardUpdateCheckInterval());
    connect(checkIntervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), &upd,
            &UpdatesSettings::setCardUpdateCheckInterval);

    auto *checkGrid = new QGridLayout;
    checkGrid->addWidget(startupBehaviorLabel, 0, 0);
    checkGrid->addWidget(startupBehaviorCombo, 0, 1);
    checkGrid->addWidget(checkIntervalLabel, 1, 0);
    checkGrid->addWidget(checkIntervalSpinBox, 1, 1);

    auto *layout = new QVBoxLayout(this);
    layout->addStretch();
    layout->addWidget(statusLabel);
    layout->addSpacing(12);
    layout->addWidget(progressBar, 0, Qt::AlignHCenter);
    layout->addSpacing(12);
    layout->addWidget(retryButton, 0, Qt::AlignHCenter);
    layout->addWidget(manualButton, 0, Qt::AlignHCenter);
    layout->addSpacing(16);
    layout->addWidget(advancedToggleButton);
    layout->addWidget(advancedPanel);
    layout->addSpacing(8);
    layout->addLayout(checkGrid);
    layout->addStretch();

    retranslateUi();
}

bool CardDatabaseSetupPage::alreadyHaveDatabase() const
{
    return CardDatabaseManager::getInstance()->getCardList().count() > 0;
}

QString CardDatabaseSetupPage::oracleSettingsFilePath() const
{
    return SettingsCache::instance().getSettingsPath() + "oracle.ini";
}

QString CardDatabaseSetupPage::readCustomUrl() const
{
    QSettings oracleSettings(oracleSettingsFilePath(), QSettings::IniFormat);
    return oracleSettings.value("allsetsurl").toString();
}

void CardDatabaseSetupPage::writeCustomUrl(const QString &url)
{
    QSettings oracleSettings(oracleSettingsFilePath(), QSettings::IniFormat);
    if (url.isEmpty()) {
        oracleSettings.remove("allsetsurl");
    } else {
        oracleSettings.setValue("allsetsurl", url);
    }
}

void CardDatabaseSetupPage::initializePage()
{
    urlLineEdit->setText(readCustomUrl());

    if (state != State::NotStarted) {
        return;
    }

    if (alreadyHaveDatabase()) {
        setState(State::Succeeded);
        return;
    }

    // Don't auto-download — wait for the user to press "Download".
    setState(State::NotStarted);
    statusLabel->setText(tr("Press Download to fetch the card database, or Skip to do it later."));
}

void CardDatabaseSetupPage::onUpdateFinished(bool success)
{
    setState(success ? State::Succeeded : State::Failed);
    if (success) {
        emit advanceRequested();
    }
}

QString CardDatabaseSetupPage::nextButtonText() const
{
    return state == State::NotStarted ? tr("Download") : QString();
}

bool CardDatabaseSetupPage::handleNextClick()
{
    if (state == State::NotStarted) {
        setState(State::Running);
        emit updateRequested();
        return false;
    }
    return true;
}

void CardDatabaseSetupPage::onToggleAdvanced(bool open)
{
    advancedToggleButton->setText(open ? tr("▼  Advanced: custom download source")
                                       : tr("▶  Advanced: custom download source"));
    advancedPanel->setVisible(open);
}

void CardDatabaseSetupPage::onApplyCustomUrl()
{
    const QString text = urlLineEdit->text().trimmed();

    if (!text.isEmpty()) {
        const QUrl url = QUrl::fromUserInput(text);
        if (!url.isValid()) {
            QMessageBox::warning(this, tr("Invalid URL"),
                                 tr("That doesn't look like a valid URL. Double-check it and try again, "
                                    "or clear the field to use the default source."));
            return;
        }
    }

    writeCustomUrl(text);
    setState(State::Running);
    emit updateRequested();
}

void CardDatabaseSetupPage::onRestoreDefaultUrl()
{
    urlLineEdit->clear();
    writeCustomUrl(QString());
}

void CardDatabaseSetupPage::setState(State newState)
{
    state = newState;

    progressBar->setVisible(state == State::Running);
    retryButton->setVisible(state == State::Failed);
    manualButton->setVisible(state == State::Failed);
    applyAndRetryButton->setEnabled(state != State::Running);

    switch (state) {
        case State::NotStarted:
            statusLabel->setText(tr("Press Download to fetch the card database, or Skip to do it later."));
            break;
        case State::Running:
            statusLabel->setText(tr("Downloading the latest card database…"));
            break;
        case State::Succeeded:
            statusLabel->setText(tr("Card database ready ✓"));
            break;
        case State::Failed:
            statusLabel->setText(
                tr("Couldn't download the card database automatically. Check your connection and retry, "
                   "set it up manually, or skip this for now — you can do it later from the Card Database menu."));
            break;
    }

    emit completeChanged();
}

bool CardDatabaseSetupPage::isComplete() const
{
    return state != State::Running;
}

bool CardDatabaseSetupPage::isSkippable() const
{
    return state != State::Succeeded;
}

QString CardDatabaseSetupPage::stepTitle() const
{
    return tr("Card Database");
}

QString CardDatabaseSetupPage::stepSubtitle() const
{
    return tr("Cockatrice needs card data to know what you're playing with.");
}

void CardDatabaseSetupPage::retranslateUi()
{
    retryButton->setText(tr("Retry"));
    manualButton->setText(tr("Set up manually…"));

    onToggleAdvanced(advancedToggleButton->isChecked());
    urlLineEdit->setPlaceholderText(tr("Leave blank to use the default source"));
    urlHintLabel->setText(tr("Only change this if you know you need a mirror or a custom card data source."));
    restoreDefaultUrlButton->setText(tr("Restore default"));
    applyAndRetryButton->setText(tr("Apply && retry"));

    startupBehaviorLabel->setText(tr("Check for card database updates on startup"));
    startupBehaviorCombo->setItemText(0, tr("Don't check"));
    startupBehaviorCombo->setItemText(1, tr("Prompt for update"));
    startupBehaviorCombo->setItemText(2, tr("Always update in the background"));

    checkIntervalLabel->setText(tr("Check for card database updates every"));
    checkIntervalSpinBox->setSuffix(tr(" days"));

    setState(state);
}
