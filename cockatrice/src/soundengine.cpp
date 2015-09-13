#include "soundengine.h"
#include "settingscache.h"

#include <QApplication>
#include <QFileInfo>
#include <QSound>
#include <QLibraryInfo>
#if QT_VERSION < 0x050000
    #include <QDesktopServices>
#else
    #include <QStandardPaths>
#endif

#define DEFAULT_THEME_NAME "Default"

/*
    fileNames = QStringList()
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
        // Chat & UI
        << "chat_mention" << "all_mention" << "private_message";
*/

#define TEST_SOUND_FILENAME "player_join"

SoundEngine::SoundEngine(QObject *parent)
: QObject(parent), engine(0)
{
    ensureThemeDirectoryExists();
    connect(settingsCache, SIGNAL(soundThemeChanged()), this, SLOT(themeChangedSlot()));
    connect(settingsCache, SIGNAL(soundEnabledChanged()), this, SLOT(soundEnabledChanged()));

    soundEnabledChanged();
    themeChangedSlot();
}

SoundEngine::~SoundEngine()
{
    if(engine)
    {
        delete engine;
        engine = 0;
    }    
}

void SoundEngine::soundEnabledChanged()
{
    if (settingsCache->getSoundEnabled()) {
#if QT_VERSION < 0x050000 //QT4
        if(QSound::isAvailable())
        {
            qDebug("SoundEngine: enabling sound");
            enabled = true;
        } else {
            qDebug("SoundEngine: sound not available");
            enabled = false;
        }
#else
        qDebug("SoundEngine: enabling sound");
        enabled = true;
#endif
    } else {
        qDebug("SoundEngine: disabling sound");
        enabled = false;
    }
}

#include <QDebug>
void SoundEngine::playSound(QString fileName)
{
    if(!enabled)
        return;

    QFileInfo fi("sounds:/" + fileName + ".wav");
    qDebug() << "playing" << fi.absoluteFilePath();    
    if(!fi.exists())
        return;

    if(engine)
    {
        if(engine->isFinished())
        {
            engine->stop();
            delete engine;            
        } else {
            return;
        }
    }

    engine = new QSound(fi.absoluteFilePath());
    engine->play();
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
#elif QT_VERSION < 0x050000
        QDesktopServices::storageLocation(QDesktopServices::DataLocation) +
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

    // resources
    QStringList resources;
    resources << dir.absolutePath();
    QDir::setSearchPaths("sounds", resources);
}