#include "soundengine.h"
#include "settingscache.h"

#include <QApplication>
#include <QAudioOutput>
#include <QBuffer>
#include <QDebug>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QStandardPaths>

#define DEFAULT_THEME_NAME "Default"
#define TEST_SOUND_FILENAME "player_join"

SoundEngine::SoundEngine(QObject *parent)
: QObject(parent), player(0)
{
    inputBuffer = new QBuffer(this);

    ensureThemeDirectoryExists();
    connect(settingsCache, SIGNAL(soundThemeChanged()), this, SLOT(themeChangedSlot()));
    connect(settingsCache, SIGNAL(soundEnabledChanged()), this, SLOT(soundEnabledChanged()));

    soundEnabledChanged();
    themeChangedSlot();
}

SoundEngine::~SoundEngine()
{
    if(player)
    {
        player->deleteLater();
        player = 0;
    }

    inputBuffer->deleteLater();
}

void SoundEngine::soundEnabledChanged()
{
    if (settingsCache->getSoundEnabled()) {
        qDebug("SoundEngine: enabling sound");
        if(!player)
        {
            QAudioFormat format;
            format.setSampleRate(44100);
            format.setChannelCount(1);
            format.setSampleSize(16);
            format.setCodec("audio/pcm");
            format.setByteOrder(QAudioFormat::LittleEndian);
            format.setSampleType(QAudioFormat::SignedInt);
            player = new QAudioOutput(format, this);
        }
    } else {
        qDebug("SoundEngine: disabling sound");
        if(player)
        {
            player->stop();
            player->deleteLater();
            player = 0;
        }
    }
}

void SoundEngine::playSound(QString fileName)
{
    if(!player)
        return;

    // still playing the previous sound?
    if(player->state() == QAudio::ActiveState)
        return;

    if(!audioData.contains(fileName))
        return;

    qDebug() << "playing" << fileName;

    inputBuffer->close();
    inputBuffer->setData(audioData[fileName]);
    inputBuffer->open(QIODevice::ReadOnly);

    player->setVolume(settingsCache->getMasterVolume() / 100.0);
    player->stop();
    player->start(inputBuffer);
}

void SoundEngine::testSound()
{
    playSound(TEST_SOUND_FILENAME);
}

void SoundEngine::ensureThemeDirectoryExists()
{
    if(settingsCache->getSoundThemeName().isEmpty() ||
        !getAvailableThemes().contains(settingsCache->getSoundThemeName()))
    {
        qDebug() << "Sounds theme name not set, setting default value";
        settingsCache->setSoundThemeName(DEFAULT_THEME_NAME);
    }
}

QStringMap & SoundEngine::getAvailableThemes()
{
    QDir dir;
    availableThemes.clear();

    // load themes from user profile dir
    dir =
#ifdef PORTABLE_BUILD
        qApp->applicationDirPath() +
#else
        QStandardPaths::standardLocations(QStandardPaths::DataLocation).first() +
#endif
        "/sounds";

    foreach(QString themeName, dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name))
    {
        if(!availableThemes.contains(themeName))
            availableThemes.insert(themeName, dir.absoluteFilePath(themeName));
    }

    // load themes from cockatrice system dir
#ifdef Q_OS_MAC
    dir = qApp->applicationDirPath() + "/../Resources/sounds";
#elif defined(Q_OS_WIN)
    dir = qApp->applicationDirPath() + "/sounds";
#else // linux
    dir = qApp->applicationDirPath() + "/../share/cockatrice/sounds";
#endif
    foreach(QString themeName, dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name))
    {
        if(!availableThemes.contains(themeName))
            availableThemes.insert(themeName, dir.absoluteFilePath(themeName));
    }

    return availableThemes;
}

void SoundEngine::themeChangedSlot()
{
    QString themeName = settingsCache->getSoundThemeName();
    qDebug() << "Sound theme changed:" << themeName;

    QDir dir = getAvailableThemes().value(themeName);

    audioData.clear();

    static const QStringList fileNames = QStringList()
        // Phases
        << "untap_step" << "upkeep_step" << "draw_step" << "main_1"
        << "start_combat" << "attack_step" << "block_step" << "damage_step" << "end_combat"
        << "main_2" << "end_step"
        // Game Actions
        << "draw_card" << "play_card" << "tap_card" << "untap_card"
        << "shuffle" << "roll_dice" << "life_change"
        // Player
        << "player_join" << "player_leave" << "player_disconnect" << "player_reconnect" << "player_concede"
        // Spectator
        << "spectator_join" << "spectator_leave"
        // Buddy
        << "buddy_join" << "buddy_leave"
        // Chat & UI
        << "chat_mention" << "all_mention" << "private_message";

    for (int i = 0; i < fileNames.size(); ++i) {
        if(!dir.exists(fileNames[i] + ".wav"))
            continue;

        QFile file(dir.filePath(fileNames[i] + ".wav"));
        file.open(QIODevice::ReadOnly);
        // 44 = length of wav header
        audioData.insert(fileNames[i], file.readAll().mid(44));
        file.close();
    }

    soundEnabledChanged();
}