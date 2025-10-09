#include "replay_manager.h"

#include "../interface/widgets/tabs/tab_game.h"

#include <QHBoxLayout>
#include <QToolButton>

ReplayManager::ReplayManager(TabGame *parent, GameReplay *_replay)
    : QWidget(parent), game(parent), replay(_replay), replayPlayButton(nullptr), replayFastForwardButton(nullptr),
      aReplaySkipForward(nullptr), aReplaySkipBackward(nullptr), aReplaySkipForwardBig(nullptr),
      aReplaySkipBackwardBig(nullptr)
{
    if (replay) {
        game->getGame()->loadReplay(replay);

        // Create list: event number -> time [ms]
        // Distribute simultaneous events evenly across 1 second.
        unsigned int lastEventTimestamp = 0;
        const int eventCount = replay->event_list_size();
        for (int i = 0; i < eventCount; ++i) {
            int j = i + 1;
            while ((j < eventCount) && (replay->event_list(j).seconds_elapsed() == lastEventTimestamp))
                ++j;

            const int numberEventsThisSecond = j - i;
            for (int k = 0; k < numberEventsThisSecond; ++k)
                replayTimeline.append(replay->event_list(i + k).seconds_elapsed() * 1000 +
                                      (int)((qreal)k / (qreal)numberEventsThisSecond * 1000));

            if (j < eventCount)
                lastEventTimestamp = replay->event_list(j).seconds_elapsed();
            i += numberEventsThisSecond - 1;
        }
    }

    // timeline widget
    timelineWidget = new ReplayTimelineWidget;
    timelineWidget->setTimeline(replayTimeline);
    connect(timelineWidget, &ReplayTimelineWidget::processNextEvent, this, &ReplayManager::replayNextEvent);
    connect(timelineWidget, &ReplayTimelineWidget::replayFinished, this, &ReplayManager::replayFinished);
    connect(timelineWidget, &ReplayTimelineWidget::rewound, this, &ReplayManager::replayRewind);

    // timeline skip shortcuts
    aReplaySkipForward = new QAction(timelineWidget);
    timelineWidget->addAction(aReplaySkipForward);
    connect(aReplaySkipForward, &QAction::triggered, this,
            [this] { timelineWidget->skipByAmount(ReplayTimelineWidget::SMALL_SKIP_MS); });

    aReplaySkipBackward = new QAction(timelineWidget);
    timelineWidget->addAction(aReplaySkipBackward);
    connect(aReplaySkipBackward, &QAction::triggered, this,
            [this] { timelineWidget->skipByAmount(-ReplayTimelineWidget::SMALL_SKIP_MS); });

    aReplaySkipForwardBig = new QAction(timelineWidget);
    timelineWidget->addAction(aReplaySkipForwardBig);
    connect(aReplaySkipForwardBig, &QAction::triggered, this,
            [this] { timelineWidget->skipByAmount(ReplayTimelineWidget::BIG_SKIP_MS); });

    aReplaySkipBackwardBig = new QAction(timelineWidget);
    timelineWidget->addAction(aReplaySkipBackwardBig);
    connect(aReplaySkipBackwardBig, &QAction::triggered, this,
            [this] { timelineWidget->skipByAmount(-ReplayTimelineWidget::BIG_SKIP_MS); });

    // buttons
    replayPlayButton = new QToolButton;
    replayPlayButton->setIconSize(QSize(32, 32));
    QIcon playButtonIcon = QIcon();
    playButtonIcon.addPixmap(QPixmap("theme:replay/start"), QIcon::Normal, QIcon::Off);
    playButtonIcon.addPixmap(QPixmap("theme:replay/pause"), QIcon::Normal, QIcon::On);
    replayPlayButton->setIcon(playButtonIcon);
    replayPlayButton->setCheckable(true);
    connect(replayPlayButton, &QToolButton::toggled, this, &ReplayManager::replayPlayButtonToggled);

    replayFastForwardButton = new QToolButton;
    replayFastForwardButton->setIconSize(QSize(32, 32));
    replayFastForwardButton->setIcon(QPixmap("theme:replay/fastforward"));
    replayFastForwardButton->setCheckable(true);
    connect(replayFastForwardButton, &QToolButton::toggled, this, &ReplayManager::replayFastForwardButtonToggled);

    // putting everything together
    auto replayControlLayout = new QHBoxLayout;
    replayControlLayout->addWidget(timelineWidget, 10);
    replayControlLayout->addWidget(replayPlayButton);
    replayControlLayout->addWidget(replayFastForwardButton);

    setObjectName("replayControlWidget");
    setLayout(replayControlLayout);

    connect(this, &ReplayManager::requestChatAndPhaseReset, game, &TabGame::resetChatAndPhase);

    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &ReplayManager::refreshShortcuts);
    refreshShortcuts();
}

void ReplayManager::replayNextEvent(EventProcessingOptions options)
{
    game->getGame()->getGameEventHandler()->processGameEventContainer(
        replay->event_list(timelineWidget->getCurrentEvent()), nullptr, options);
}

void ReplayManager::replayFinished()
{
    replayPlayButton->setChecked(false);
}

void ReplayManager::replayPlayButtonToggled(bool checked)
{
    if (checked) { // start replay
        timelineWidget->startReplay();
    } else { // pause replay
        timelineWidget->stopReplay();
    }
}

void ReplayManager::replayFastForwardButtonToggled(bool checked)
{
    timelineWidget->setTimeScaleFactor(checked ? ReplayTimelineWidget::FAST_FORWARD_SCALE_FACTOR : 1.0);
}

/**
 * @brief Handles everything that needs to be reset when doing a replay rewind.
 */
void ReplayManager::replayRewind()
{
    emit requestChatAndPhaseReset();
}

void ReplayManager::refreshShortcuts()
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