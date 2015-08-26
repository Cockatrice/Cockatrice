#include "soundengine.h"
#include "settingscache.h"
#include <QAudioOutput>
#include <QAudioFormat>
#include <QFile>
#include <QBuffer>
#include <QStringList>
#if QT_VERSION > 0x050000
    #include <QMediaPlayer>
    #include <QUrl>
#endif

SoundEngine::SoundEngine(QObject *parent)
: QObject(parent), audio(0)
{
    enabled = false;
    inputBuffer = new QBuffer(this);
    
    connect(settingsCache, SIGNAL(soundPathChanged()), this, SLOT(cacheData()));
    connect(settingsCache, SIGNAL(soundEnabledChanged()), this, SLOT(soundEnabledChanged()));
    cacheData();
    soundEnabledChanged();
}

void SoundEngine::cacheData()
{
    fileNames = QStringList()
        /*Phases*/
        << "untap_step" << "upkeep_step" << "draw_step" << "main_1"
        << "start_combat" << "attack_step" << "block_step" << "damage_step" << "end_combat"
        << "main_2" << "end_step"
        /*Game Actions*/
        << "draw_card" << "play_card" << "tap_card" << "untap_card"
        << "shuffle" << "roll_dice" << "life_change"
        /*Player*/
        << "player_join" << "player_leave" << "player_disconnect" << "player_reconnect" << "player_concede"
        /*Spectator*/
        << "spectator_join" << "spectator_leave"
        /*Chat & UI*/
        << "chat_mention" << "all_mention" << "private_message";

    #if QT_VERSION < 0x050000 //QT4
        for (int i = 0; i < fileNames.size(); ++i) {
            QFile file(settingsCache->getSoundPath() + "/" + fileNames[i] + ".wav");
            if(!file.exists())
                continue;
            file.open(QIODevice::ReadOnly);
            file.seek(44);
            audioCache.insert(fileNames[i], file.readAll());
            file.close();
        }
    #else //QT5
        QStringList ext = QStringList() << ".mp4" << ".mp3" << ".wav";
        for (int i = 0; i < fileNames.size(); ++i) {
            for (int j = 0; j < ext.size(); ++j) {
                QString filepath = settingsCache->getSoundPath() + "/" + fileNames[i] + ext[j];
                QFile file(filepath);
                if (file.exists()){
                    QMediaPlayer *player = new QMediaPlayer;
                    player->setMedia(QUrl::fromLocalFile(filepath));
                    audioData.insert(fileNames[i], player);
                    break;
                }
            }
        }
    #endif
}

void SoundEngine::soundEnabledChanged()
{
#if QT_VERSION < 0x050000 //QT4
    if (settingsCache->getSoundEnabled()) {
        qDebug("SoundEngine: enabling sound");
        QAudioFormat format;
        format.setSampleRate(44100);
        format.setChannelCount(1);
        format.setSampleSize(16);
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setSampleType(QAudioFormat::SignedInt);
        audio = new QAudioOutput(format, this);
    } else if (audio) {
        qDebug("SoundEngine: disabling sound");
        audio->stop();
        audio->deleteLater();
        audio = 0;
    }
#else //QT5
    if (settingsCache->getSoundEnabled()) {
        qDebug("SoundEngine: enabling sound");
        enabled = true;
    }else{
        qDebug("SoundEngine: disabling sound");
        enabled = false;
        for (int i = 0; i < fileNames.size(); ++i) {
            if (audioData.contains(fileNames[i]) && 
                audioData[fileNames[i]]->state() != QMediaPlayer::StoppedState)
                    audioData[fileNames[i]]->stop();
       }
    }
#endif
}

void SoundEngine::playSound(QString fileName)
{
#if QT_VERSION < 0x050000 //QT4
    if (!fileNames.contains(fileName) || !audio)
        return;
    
    audio->stop();
    inputBuffer->close();
    inputBuffer->setData(audioCache[fileName]);
    inputBuffer->open(QIODevice::ReadOnly);
    audio->start(inputBuffer);
#else //QT5
    if (!audioData.contains(fileName) || !enabled){
        return;
    }

    audioData[fileName]->setVolume(settingsCache->getMasterVolume());
    audioData[fileName]->stop();
    audioData[fileName]->setPosition(0);
    audioData[fileName]->play();
#endif
}