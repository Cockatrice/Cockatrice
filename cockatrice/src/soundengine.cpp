#include "soundengine.h"
#include "settingscache.h"

#include <QFileInfo>
#include <QSound>

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
: QObject(parent), enabled(false)
{
    connect(settingsCache, SIGNAL(soundEnabledChanged()), this, SLOT(soundEnabledChanged()));

    soundEnabledChanged();
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

    QFileInfo fi(settingsCache->getSoundPath() + "/" + fileName + ".wav");
    qDebug() << "playing" << fi.absoluteFilePath();    
    if(!fi.exists())
        return;

    QSound::play(fi.absoluteFilePath());
}

void SoundEngine::testSound()
{
    playSound(TEST_SOUND_FILENAME);
}