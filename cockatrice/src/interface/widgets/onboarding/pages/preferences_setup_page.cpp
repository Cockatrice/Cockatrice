#include "preferences_setup_page.h"

#include "../../client/settings/cache_settings.h"
#include "../../client/sound_engine.h"
#include "libcockatrice/settings/interface_settings.h"
#include "libcockatrice/settings/sound_settings.h"
#include "libcockatrice/settings/tabs_settings.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

namespace
{
// The server destinations are omitted: during first run their tabs are not
// open yet, and the wizard offers no way to fill in the server/room details.
QList<StartupTab> wizardStartupTabOrder()
{
    return {StartupTabHome,    StartupTabVisualDeckStorage, StartupTabDeckStorage,
            StartupTabReplays, StartupTabDeckEditor,        StartupTabVisualDeckEditor};
}
} // namespace

PreferencesSetupPage::PreferencesSetupPage(QWidget *parent) : FirstRunWizardPage(parent)
{
    auto *content = new QWidget;
    auto *contentLayout = new QVBoxLayout(content);

    gameplayGroup = new QGroupBox(content);
    auto *gameplayLayout = new QVBoxLayout(gameplayGroup);
    contentLayout->addWidget(gameplayGroup);
    doubleClickToPlayCheckBox = new QCheckBox(gameplayGroup);
    horizontalHandCheckBox = new QCheckBox(gameplayGroup);
    playToStackCheckBox = new QCheckBox(gameplayGroup);
    gameplayLayout->addWidget(doubleClickToPlayCheckBox);
    gameplayLayout->addWidget(horizontalHandCheckBox);
    gameplayLayout->addWidget(playToStackCheckBox);

    notificationsGroup = new QGroupBox(content);
    auto *notificationsLayout = new QVBoxLayout(notificationsGroup);
    contentLayout->addWidget(notificationsGroup);
    notificationsEnabledCheckBox = new QCheckBox(notificationsGroup);
    soundEnabledCheckBox = new QCheckBox(notificationsGroup);
    notificationsLayout->addWidget(notificationsEnabledCheckBox);
    notificationsLayout->addWidget(soundEnabledCheckBox);

    startupGroup = new QGroupBox(content);
    auto *startupForm = new QFormLayout(startupGroup);
    contentLayout->addWidget(startupGroup);
    startupTabLabel = new QLabel(startupGroup);
    startupTabSelector = new QComboBox(startupGroup);
    startupTabSelector->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    for (StartupTab tab : wizardStartupTabOrder()) {
        startupTabSelector->addItem(QString(), tab); // texts set in retranslateUi
    }
    startupForm->addRow(startupTabLabel, startupTabSelector);

    contentLayout->addStretch();

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(content);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(scrollArea);

    SettingsCache &settings = SettingsCache::instance();

    connect(doubleClickToPlayCheckBox, &QCheckBox::toggled, &settings.userInterface(),
            &InterfaceSettings::setDoubleClickToPlay);
    connect(horizontalHandCheckBox, &QCheckBox::toggled, &settings.userInterface(),
            &InterfaceSettings::setHorizontalHand);
    connect(playToStackCheckBox, &QCheckBox::toggled, &settings.userInterface(), &InterfaceSettings::setPlayToStack);

    connect(notificationsEnabledCheckBox, &QCheckBox::toggled, &settings.userInterface(),
            &InterfaceSettings::setNotificationsEnabled);
    connect(soundEnabledCheckBox, &QCheckBox::toggled, &settings.sound(), &SoundSettings::setSoundEnabled);
    connect(soundEnabledCheckBox, &QCheckBox::toggled, soundEngine, &SoundEngine::testSound);

    connect(startupTabSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (index < 0) {
            return;
        }
        SettingsCache::instance().tabs().setStartupTabIndex(startupTabSelector->itemData(index).toInt());
    });

    retranslateUi();
}

void PreferencesSetupPage::initializePage()
{
    SettingsCache &settings = SettingsCache::instance();

    doubleClickToPlayCheckBox->setChecked(settings.userInterface().getDoubleClickToPlay());
    horizontalHandCheckBox->setChecked(settings.userInterface().getHorizontalHand());
    playToStackCheckBox->setChecked(settings.userInterface().getPlayToStack());

    notificationsEnabledCheckBox->setChecked(settings.userInterface().getNotificationsEnabled());
    soundEnabledCheckBox->setChecked(settings.sound().getSoundEnabled());

    startupTabSelector->setCurrentIndex(startupTabSelector->findData(settings.tabs().getStartupTabIndex()));
}

bool PreferencesSetupPage::isSkippable() const
{
    return true;
}

QString PreferencesSetupPage::stepTitle() const
{
    return tr("A Few Preferences");
}

QString PreferencesSetupPage::stepSubtitle() const
{
    return tr("Defaults are fine — tweak these now or from Settings anytime.");
}

void PreferencesSetupPage::retranslateUi()
{
    gameplayGroup->setTitle(tr("Gameplay"));
    doubleClickToPlayCheckBox->setText(tr("Double-click cards to play them"));
    doubleClickToPlayCheckBox->setToolTip(tr("When disabled, a single click plays the selected card onto the table."));
    horizontalHandCheckBox->setText(tr("Display hand horizontally"));
    horizontalHandCheckBox->setToolTip(
        tr("Shows your hand as a row along the bottom of the table instead of a column beside it."));
    playToStackCheckBox->setText(tr("Play all nonlands onto the stack by default"));
    playToStackCheckBox->setToolTip(
        tr("Cards you play appear on the stack so other players can respond to them, as in a tabletop game."));

    notificationsGroup->setTitle(tr("Notifications && Sound"));
    notificationsEnabledCheckBox->setText(tr("Show desktop notifications"));
    soundEnabledCheckBox->setText(tr("Play sound effects"));

    startupGroup->setTitle(tr("Startup"));
    startupTabLabel->setText(tr("Startup tab:"));
    const QList<StartupTab> tabs = wizardStartupTabOrder();
    for (int i = 0; i < tabs.size(); ++i) {
        QString name;
        switch (tabs[i]) {
            case StartupTabHome:
                name = tr("Home");
                break;
            case StartupTabVisualDeckStorage:
                name = tr("Visual Deck Storage");
                break;
            case StartupTabDeckStorage:
                name = tr("Deck Storage");
                break;
            case StartupTabReplays:
                name = tr("Game Replays");
                break;
            case StartupTabDeckEditor:
                name = tr("Deck Editor");
                break;
            case StartupTabVisualDeckEditor:
                name = tr("Visual Deck Editor");
                break;
            case StartupTabServer:
                name = tr("Server");
                break;
            case StartupTabServerRoom:
                name = tr("Server Room");
                break;
        }
        startupTabSelector->setItemText(i, name);
    }
}
