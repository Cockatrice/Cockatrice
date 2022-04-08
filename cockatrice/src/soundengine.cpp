#include "soundengine.h"

#include "settingscache.h"

#include <QApplication>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QBuffer>
#include <QFileInfo>
#include <QStandardPaths>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QAudioDevice>
#include <QMediaPlayer>
#endif

#define DEFAULT_THEME_NAME "Default"
#define TEST_SOUND_FILENAME "player_join"

SoundEngine::SoundEngine(QObject *parent) : QObject(parent), player(0)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    inputBuffer = new QBuffer();
#else
    inputBuffer = new QBuffer(this);
#endif

    ensureThemeDirectoryExists();
    connect(&SettingsCache::instance(), SIGNAL(soundThemeChanged()), this, SLOT(themeChangedSlot()));
    connect(&SettingsCache::instance(), SIGNAL(soundEnabledChanged()), this, SLOT(soundEnabledChanged()));

    soundEnabledChanged();
    themeChangedSlot();
}

SoundEngine::~SoundEngine()
{
    if (player) {
        player->deleteLater();
        player = 0;
    }

    inputBuffer->deleteLater();
}

void SoundEngine::soundEnabledChanged()
{
    if (SettingsCache::instance().getSoundEnabled()) {
        qDebug("SoundEngine: enabling sound");
        if (!player) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            player = new QMediaPlayer;
            auto qAudioOutput = new QAudioOutput;
            player->setAudioOutput(qAudioOutput);
#else
            QAudioFormat format;
            format.setSampleRate(44100);
            format.setChannelCount(1);
            format.setSampleSize(16);
            format.setCodec("audio/pcm");
            format.setByteOrder(QAudioFormat::LittleEndian);
            format.setSampleType(QAudioFormat::SignedInt);
            player = new QAudioOutput(format, this);
#endif
        }
    } else {
        qDebug("SoundEngine: disabling sound");
        if (player) {
            player->stop();
            player->deleteLater();
            player = 0;
        }
    }
}

void SoundEngine::playSound(QString fileName)
{
    if (!player) {
        return;
    }

    // still playing the previous sound?
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    if (player->playbackState() == QMediaPlayer::PlaybackState::PlayingState) {
        return;
    }
#else
    if (player->state() == QAudio::ActiveState) {
        return;
    }
#endif

    if (!audioData.contains(fileName)) {
        return;
    }

    qDebug() << "playing" << fileName;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    player->audioOutput()->setVolume(SettingsCache::instance().getMasterVolume() / 100.0);
    player->stop();
    player->setSource(QUrl::fromLocalFile(audioData[fileName]));
    player->play();
#else
    inputBuffer->close();
    inputBuffer->setData(audioData[fileName]);
    inputBuffer->open(QIODevice::ReadOnly);

    player->setVolume(SettingsCache::instance().getMasterVolume() / 100.0);
    player->stop();
    player->start(inputBuffer);
#endif
}

void SoundEngine::testSound()
{
    playSound(TEST_SOUND_FILENAME);
}

void SoundEngine::ensureThemeDirectoryExists()
{
    if (SettingsCache::instance().getSoundThemeName().isEmpty() ||
        !getAvailableThemes().contains(SettingsCache::instance().getSoundThemeName())) {
        qDebug() << "Sounds theme name not set, setting default value";
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
    qDebug() << "Sound theme changed:" << themeName;

    QDir dir = getAvailableThemes().value(themeName);

    audioData.clear();

    static const QStringList fileNames = QStringList()
                                         // Phases
                                         << "untap_step"
                                         << "upkeep_step"
                                         << "draw_step"
                                         << "main_1"
                                         << "start_combat"
                                         << "attack_step"
                                         << "block_step"
                                         << "damage_step"
                                         << "end_combat"
                                         << "main_2"
                                         << "end_step"
                                         // Game Actions
                                         << "draw_card"
                                         << "play_card"
                                         << "tap_card"
                                         << "untap_card"
                                         << "shuffle"
                                         << "roll_dice"
                                         << "life_change"
                                         // Player
                                         << "player_join"
                                         << "player_leave"
                                         << "player_disconnect"
                                         << "player_reconnect"
                                         << "player_concede"
                                         // Spectator
                                         << "spectator_join"
                                         << "spectator_leave"
                                         // Buddy
                                         << "buddy_join"
                                         << "buddy_leave"
                                         // Chat & UI
                                         << "chat_mention"
                                         << "all_mention"
                                         << "private_message";

    for (int i = 0; i < fileNames.size(); ++i) {
        if (!dir.exists(fileNames[i] + ".wav"))
            continue;

        QFile file(dir.filePath(fileNames[i] + ".wav"));
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        audioData.insert(fileNames[i], file.fileName());
#else
        file.open(QIODevice::ReadOnly);
        // 44 = length of wav header
        audioData.insert(fileNames[i], file.readAll().mid(44));
        file.close();
#endif
    }

    soundEnabledChanged();
}
