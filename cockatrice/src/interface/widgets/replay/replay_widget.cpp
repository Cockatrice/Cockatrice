#include "replay_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "../../../client/settings/shortcuts_settings.h"
#include "../interface/widgets/tabs/tab_game.h"
#include "replay_manager.h"
#include "replay_quick_settings_widget.h"

#include <QHBoxLayout>
#include <QToolButton>

ReplayWidget::ReplayWidget(QWidget *parent, GameReplay *replay)
    : QWidget(parent), replayPlayButton(nullptr), replayFastForwardButton(nullptr), aReplaySkipForward(nullptr),
      aReplaySkipBackward(nullptr), aReplaySkipForwardBig(nullptr), aReplaySkipBackwardBig(nullptr)
{
    // replay manager
    replayManager = new ReplayManager(this, replay);
    connect(replayManager, &ReplayManager::eventReplayed, this, &ReplayWidget::eventReplayed);
    connect(replayManager, &ReplayManager::replayFinished, this, &ReplayWidget::replayFinished);
    connect(replayManager, &ReplayManager::rewound, this, &ReplayWidget::rewound);

    // timeline widget
    timelineWidget = new ReplayTimelineWidget;
    timelineWidget->setTimeline(replayManager->getReplayTimeline());
    connect(replayManager, &ReplayManager::timeChanged, timelineWidget, &ReplayTimelineWidget::setCurrentTime);
    connect(timelineWidget, &ReplayTimelineWidget::timeClicked, replayManager, &ReplayManager::setTime);

    // timeline skip shortcuts
    aReplaySkipForward = new QAction(timelineWidget);
    timelineWidget->addAction(aReplaySkipForward);
    connect(aReplaySkipForward, &QAction::triggered, this,
            [this] { replayManager->skipByAmount(ReplayManager::SMALL_SKIP_MS); });

    aReplaySkipBackward = new QAction(timelineWidget);
    timelineWidget->addAction(aReplaySkipBackward);
    connect(aReplaySkipBackward, &QAction::triggered, this,
            [this] { replayManager->skipByAmount(-ReplayManager::SMALL_SKIP_MS); });

    aReplaySkipForwardBig = new QAction(timelineWidget);
    timelineWidget->addAction(aReplaySkipForwardBig);
    connect(aReplaySkipForwardBig, &QAction::triggered, this,
            [this] { replayManager->skipByAmount(ReplayManager::BIG_SKIP_MS); });

    aReplaySkipBackwardBig = new QAction(timelineWidget);
    timelineWidget->addAction(aReplaySkipBackwardBig);
    connect(aReplaySkipBackwardBig, &QAction::triggered, this,
            [this] { replayManager->skipByAmount(-ReplayManager::BIG_SKIP_MS); });

    // buttons
    replayPlayButton = new QToolButton;
    replayPlayButton->setIconSize(QSize(32, 32));
    QIcon playButtonIcon = QIcon();
    playButtonIcon.addPixmap(QPixmap("theme:replay/start"), QIcon::Normal, QIcon::Off);
    playButtonIcon.addPixmap(QPixmap("theme:replay/pause"), QIcon::Normal, QIcon::On);
    replayPlayButton->setIcon(playButtonIcon);
    replayPlayButton->setCheckable(true);
    connect(replayPlayButton, &QToolButton::toggled, this, &ReplayWidget::replayPlayButtonToggled);

    replayFastForwardButton = new QToolButton;
    replayFastForwardButton->setIconSize(QSize(32, 32));
    replayFastForwardButton->setIcon(QPixmap("theme:replay/fastforward"));
    replayFastForwardButton->setCheckable(true);
    connect(replayFastForwardButton, &QToolButton::toggled, this, &ReplayWidget::updateTimeScaleFactor);

    settingsWidget = new ReplayQuickSettingsWidget(this);
    settingsWidget->setFixedSize(QSize(32, 32));
    connect(settingsWidget, &ReplayQuickSettingsWidget::fastForwardSpeedChanged, this,
            [this] { updateTimeScaleFactor(replayFastForwardButton->isChecked()); });
    connect(settingsWidget, &ReplayQuickSettingsWidget::skipEmptySectionsChanged, replayManager,
            &ReplayManager::setSkipEmptySections);

    replayManager->setSkipEmptySections(SettingsCache::instance().interface().getSkipEmptySections());

    // putting everything together
    auto replayControlLayout = new QHBoxLayout;
    replayControlLayout->addWidget(timelineWidget, 10);
    replayControlLayout->addWidget(replayPlayButton);
    replayControlLayout->addWidget(replayFastForwardButton);
    replayControlLayout->addWidget(settingsWidget);

    setObjectName("replayControlWidget");
    setLayout(replayControlLayout);

    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &ReplayWidget::refreshShortcuts);
    refreshShortcuts();
}

void ReplayWidget::replayFinished()
{
    replayPlayButton->setChecked(false);
}

void ReplayWidget::replayPlayButtonToggled(bool checked)
{
    if (checked) { // start replay
        replayManager->startReplay();
    } else { // pause replay
        replayManager->stopReplay();
    }
}

void ReplayWidget::updateTimeScaleFactor(bool isFastForward)
{
    qreal factor = isFastForward ? SettingsCache::instance().interface().getFastForwardSpeed() : 1.0;
    replayManager->setTimeScaleFactor(factor);
}

void ReplayWidget::refreshShortcuts()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();
    if (aReplaySkipForward) {
        aReplaySkipForward->setShortcuts(shortcuts.getShortcut("Replays/aSkipForward"));
    }
    if (aReplaySkipBackward) {
        aReplaySkipBackward->setShortcuts(shortcuts.getShortcut("Replays/aSkipBackward"));
    }
    if (aReplaySkipForwardBig) {
        aReplaySkipForwardBig->setShortcuts(shortcuts.getShortcut("Replays/aSkipForwardBig"));
    }
    if (aReplaySkipBackwardBig) {
        aReplaySkipBackwardBig->setShortcuts(shortcuts.getShortcut("Replays/aSkipBackwardBig"));
    }
    if (replayPlayButton) {
        replayPlayButton->setShortcut(shortcuts.getSingleShortcut("Replays/playButton"));
    }
    if (replayFastForwardButton) {
        replayFastForwardButton->setShortcut(shortcuts.getSingleShortcut("Replays/fastForwardButton"));
    }
}