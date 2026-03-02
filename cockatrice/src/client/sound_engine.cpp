#include "sound_engine.h"

#include "settings/cache_settings.h"

#include <QDir>
#include <QMediaPlayer>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QAudioOutput>
#endif

#define DEFAULT_THEME_NAME "Default"
#define TEST_SOUND_FILENAME "player_join"

SoundEngine::SoundEngine(QObject *parent) : QObject(parent), audioOutput(nullptr), player(nullptr)
{
    ensureThemeDirectoryExists();
    connect(&SettingsCache::instance(), &SettingsCache::soundThemeChanged, this, &SoundEngine::themeChangedSlot);
    connect(&SettingsCache::instance(), &SettingsCache::soundEnabledChanged, this, &SoundEngine::soundEnabledChanged);

    soundEnabledChanged();
    themeChangedSlot();
}

SoundEngine::~SoundEngine()
{
    if (player) {
        player->deleteLater();
        player = nullptr;
    }
    if (audioOutput) {
        audioOutput->deleteLater();
        audioOutput = nullptr;
    }
}

void SoundEngine::soundEnabledChanged()
{
    if (SettingsCache::instance().getSoundEnabled()) {
        qCInfo(SoundEngineLog) << "SoundEngine: enabling sound with" << audioData.size() << "sounds";
        if (!player) {
            player = new QMediaPlayer;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            audioOutput = new QAudioOutput(player);
            player->setAudioOutput(audioOutput);
#endif
        }
    } else {
        qCInfo(SoundEngineLog) << "SoundEngine: disabling sound";
        if (player) {
            player->stop();
            player->deleteLater();
            player = nullptr;
        }
        if (audioOutput) {
            audioOutput->deleteLater();
            audioOutput = nullptr;
        }
    }
}

void SoundEngine::playSound(const QString &fileName)
{
    if (!player) {
        return;
    }

    if (!audioData.contains(fileName)) {
        return;
    }

    player->stop();
    int volumeSliderValue = SettingsCache::instance().getMasterVolume();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    player->audioOutput()->setVolume(qreal(volumeSliderValue) / 100);
    player->setSource(QUrl::fromLocalFile(audioData[fileName]));
#else
    player->setVolume(volumeSliderValue);
    player->setMedia(QUrl::fromLocalFile(audioData[fileName]));
#endif
    player->play();
}

void SoundEngine::testSound()
{
    playSound(TEST_SOUND_FILENAME);
}

void SoundEngine::ensureThemeDirectoryExists()
{
    if (SettingsCache::instance().getSoundThemeName().isEmpty() ||
        !getAvailableThemes().contains(SettingsCache::instance().getSoundThemeName())) {
        qCInfo(SoundEngineLog) << "Sounds theme name not set, setting default value";
        SettingsCache::instance().setSoundThemeName(DEFAULT_THEME_NAME);
    }
}

QStringMap &SoundEngine::getAvailableThemes()
{
    QDir dir;
    availableThemes.clear();

    // load themes from user profile dir

    dir.setPath(SettingsCache::instance().getDataPath() + "/sounds");

    for (const QString &themeName : dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name)) {
        if (!availableThemes.contains(themeName))
            availableThemes.insert(themeName, dir.absoluteFilePath(themeName));
    }

    // load themes from cockatrice system dir
    dir.setPath(qApp->applicationDirPath() +
#ifdef Q_OS_MAC
                "/../Resources/sounds"
#elif defined(Q_OS_WIN)
                "/sounds"
#else // linux
                "/../share/cockatrice/sounds"
#endif
    );

    for (const QString &themeName : dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name)) {
        if (!availableThemes.contains(themeName))
            availableThemes.insert(themeName, dir.absoluteFilePath(themeName));
    }

    return availableThemes;
}

void SoundEngine::themeChangedSlot()
{
    QString themeName = SettingsCache::instance().getSoundThemeName();
    qCInfo(SoundEngineLog) << "Sound theme changed:" << themeName;

    QDir dir = getAvailableThemes().value(themeName);

    audioData.clear();

    static const QStringList extensions = {".wav", ".mp3", ".ogg"};
    static const QStringList fileNames = {
        // Phases
        "untap_step", "upkeep_step", "draw_step", "main_1", "start_combat", "attack_step", "block_step", "damage_step",
        "end_combat", "main_2", "end_step",
        // Game Actions
        "draw_card", "play_card", "tap_card", "untap_card", "shuffle", "roll_dice", "life_change",
        // Player
        "player_join", "player_leave", "player_disconnect", "player_reconnect", "player_concede",
        // Spectator
        "spectator_join", "spectator_leave",
        // Buddy
        "buddy_join", "buddy_leave",
        // Chat & UI
        "chat_mention", "all_mention", "private_message"};

    for (const QString &extension : extensions) {
        for (const QString &name : fileNames) {
            QFile file(dir.filePath(name + extension));
            if (file.exists()) {
                audioData.insert(name, file.fileName());
            }
        }
    }

    soundEnabledChanged();
}
