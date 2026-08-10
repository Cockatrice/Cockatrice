#include "preferences_setup_page.h"

#include "../../client/settings/cache_settings.h"
#include "libcockatrice/settings/appearance_settings.h"
#include "libcockatrice/settings/download_settings.h"
#include "libcockatrice/settings/network_settings.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QScrollArea>
#include <QVBoxLayout>
#include <libcockatrice/settings/cards_display_settings.h>
#include <libcockatrice/settings/interface_settings.h>
#include <libcockatrice/settings/personal_settings.h>
#include <libcockatrice/settings/sound_settings.h>
#include <libcockatrice/settings/updates_settings.h>

namespace
{
QGroupBox *makeGroup(QWidget *parent, QVBoxLayout *outerLayout)
{
    auto *group = new QGroupBox(parent);
    new QVBoxLayout(group);
    outerLayout->addWidget(group);
    return group;
}
} // namespace

PreferencesSetupPage::PreferencesSetupPage(QWidget *parent) : FirstRunWizardPage(parent)
{
    auto *content = new QWidget;
    auto *contentLayout = new QVBoxLayout(content);

    appearanceGroup = makeGroup(content, contentLayout);
    styleUserListCheckBox = new QCheckBox(appearanceGroup);
    cardScalingCheckBox = new QCheckBox(appearanceGroup);
    roundCardCornersCheckBox = new QCheckBox(appearanceGroup);
    displayCardNamesCheckBox = new QCheckBox(appearanceGroup);
    autoRotateCardsCheckBox = new QCheckBox(appearanceGroup);
    tapAnimationCheckBox = new QCheckBox(appearanceGroup);
    appearanceGroup->layout()->addWidget(styleUserListCheckBox);
    appearanceGroup->layout()->addWidget(cardScalingCheckBox);
    appearanceGroup->layout()->addWidget(roundCardCornersCheckBox);
    appearanceGroup->layout()->addWidget(displayCardNamesCheckBox);
    appearanceGroup->layout()->addWidget(autoRotateCardsCheckBox);
    appearanceGroup->layout()->addWidget(tapAnimationCheckBox);

    notificationsGroup = makeGroup(content, contentLayout);
    notificationsEnabledCheckBox = new QCheckBox(notificationsGroup);
    soundEnabledCheckBox = new QCheckBox(notificationsGroup);
    notificationsGroup->layout()->addWidget(notificationsEnabledCheckBox);
    notificationsGroup->layout()->addWidget(soundEnabledCheckBox);

    gameplayGroup = makeGroup(content, contentLayout);
    doubleClickToPlayCheckBox = new QCheckBox(gameplayGroup);
    horizontalHandCheckBox = new QCheckBox(gameplayGroup);
    playToStackCheckBox = new QCheckBox(gameplayGroup);
    gameplayGroup->layout()->addWidget(doubleClickToPlayCheckBox);
    gameplayGroup->layout()->addWidget(horizontalHandCheckBox);
    gameplayGroup->layout()->addWidget(playToStackCheckBox);

    menuGroup = makeGroup(content, contentLayout);
    showShortcutsCheckBox = new QCheckBox(menuGroup);
    menuGroup->layout()->addWidget(showShortcutsCheckBox);

    dataGroup = makeGroup(content, contentLayout);
    picDownloadCheckBox = new QCheckBox(dataGroup);
    checkUpdatesOnStartupCheckBox = new QCheckBox(dataGroup);
    showTipsOnStartupCheckBox = new QCheckBox(dataGroup);
    dataGroup->layout()->addWidget(picDownloadCheckBox);
    dataGroup->layout()->addWidget(checkUpdatesOnStartupCheckBox);
    dataGroup->layout()->addWidget(showTipsOnStartupCheckBox);

    contentLayout->addStretch();

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(content);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(scrollArea);

    SettingsCache &settings = SettingsCache::instance();

    connect(styleUserListCheckBox, &QCheckBox::toggled, &settings.appearance(), &AppearanceSettings::setStyleUserList);
    connect(cardScalingCheckBox, &QCheckBox::toggled, &settings.cardsDisplay(), &CardsDisplaySettings::setCardScaling);
    connect(roundCardCornersCheckBox, &QCheckBox::toggled, &settings.cardsDisplay(),
            &CardsDisplaySettings::setRoundCardCorners);
    connect(displayCardNamesCheckBox, &QCheckBox::toggled, &settings.cardsDisplay(),
            &CardsDisplaySettings::setDisplayCardNames);
    connect(autoRotateCardsCheckBox, &QCheckBox::toggled, &settings.cardsDisplay(),
            &CardsDisplaySettings::setAutoRotateSidewaysLayoutCards);
    connect(tapAnimationCheckBox, &QCheckBox::toggled, &settings.cardsDisplay(),
            &CardsDisplaySettings::setTapAnimation);

    connect(notificationsEnabledCheckBox, &QCheckBox::toggled, &settings.userInterface(),
            &InterfaceSettings::setNotificationsEnabled);
    connect(soundEnabledCheckBox, &QCheckBox::toggled, &settings.sound(), &SoundSettings::setSoundEnabled);

    connect(doubleClickToPlayCheckBox, &QCheckBox::toggled, &settings.userInterface(),
            &InterfaceSettings::setDoubleClickToPlay);
    connect(horizontalHandCheckBox, &QCheckBox::toggled, &settings.userInterface(),
            &InterfaceSettings::setHorizontalHand);
    connect(playToStackCheckBox, &QCheckBox::toggled, &settings.userInterface(), &InterfaceSettings::setPlayToStack);

    connect(showShortcutsCheckBox, &QCheckBox::toggled, &settings.userInterface(),
            &InterfaceSettings::setShowShortcuts);

    connect(picDownloadCheckBox, &QCheckBox::toggled, &settings.downloads(), &DownloadSettings::setPicDownload);
    connect(checkUpdatesOnStartupCheckBox, &QCheckBox::toggled, &settings.updates(),
            &UpdatesSettings::setCheckUpdatesOnStartup);
    connect(showTipsOnStartupCheckBox, &QCheckBox::toggled, &settings.personal(),
            &PersonalSettings::setShowTipsOnStartup);

    retranslateUi();
}

void PreferencesSetupPage::initializePage()
{
    SettingsCache &settings = SettingsCache::instance();

    styleUserListCheckBox->setChecked(settings.appearance().getStyleUserList());
    cardScalingCheckBox->setChecked(settings.cardsDisplay().getScaleCards());
    roundCardCornersCheckBox->setChecked(settings.cardsDisplay().getRoundCardCorners());
    displayCardNamesCheckBox->setChecked(settings.cardsDisplay().getDisplayCardNames());
    autoRotateCardsCheckBox->setChecked(settings.cardsDisplay().getAutoRotateSidewaysLayoutCards());
    tapAnimationCheckBox->setChecked(settings.cardsDisplay().getTapAnimation());

    notificationsEnabledCheckBox->setChecked(settings.userInterface().getNotificationsEnabled());
    soundEnabledCheckBox->setChecked(settings.sound().getSoundEnabled());

    doubleClickToPlayCheckBox->setChecked(settings.userInterface().getDoubleClickToPlay());
    horizontalHandCheckBox->setChecked(settings.userInterface().getHorizontalHand());
    playToStackCheckBox->setChecked(settings.userInterface().getPlayToStack());

    showShortcutsCheckBox->setChecked(settings.userInterface().getShowShortcuts());

    picDownloadCheckBox->setChecked(settings.downloads().getPicDownload());
    checkUpdatesOnStartupCheckBox->setChecked(settings.updates().getCheckUpdatesOnStartup());
    showTipsOnStartupCheckBox->setChecked(settings.personal().getShowTipsOnStartup());
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
    appearanceGroup->setTitle(tr("Appearance"));
    styleUserListCheckBox->setText(tr("Use the styled user list (avatars, role colours)"));
    cardScalingCheckBox->setText(tr("Scale cards to fit the window"));
    roundCardCornersCheckBox->setText(tr("Round card corners"));
    displayCardNamesCheckBox->setText(tr("Display card names on pictured cards"));
    autoRotateCardsCheckBox->setText(tr("Auto-rotate sideways layout cards"));
    tapAnimationCheckBox->setText(tr("Animate tapping cards"));

    notificationsGroup->setTitle(tr("Notifications && Sound"));
    notificationsEnabledCheckBox->setText(tr("Show desktop notifications"));
    soundEnabledCheckBox->setText(tr("Play sound effects"));

    gameplayGroup->setTitle(tr("Gameplay"));
    doubleClickToPlayCheckBox->setText(tr("Double-click a card to play it"));
    horizontalHandCheckBox->setText(tr("Display hand horizontally"));
    playToStackCheckBox->setText(tr("Play cards to top of stack"));

    menuGroup->setTitle(tr("Menus"));
    showShortcutsCheckBox->setText(tr("Show keyboard shortcuts in menus"));

    dataGroup->setTitle(tr("Updates && Data"));
    picDownloadCheckBox->setText(tr("Automatically download card images"));
    picDownloadCheckBox->setToolTip(tr("Turn this off if you're on a limited connection — "
                                       "card art just won't load until you turn it back on."));
    checkUpdatesOnStartupCheckBox->setText(tr("Check for client updates on startup"));
    showTipsOnStartupCheckBox->setText(tr("Show tip of the day on startup"));
}