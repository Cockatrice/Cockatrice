#include "soundengine.h"
#include "settingscache.h"
#include <QAudioOutput>
#include <QAudioFormat>
#include <QFile>
#include <QBuffer>

SoundEngine::SoundEngine(QObject *parent)
    : QObject(parent), audio(0)
{
    inputBuffer = new QBuffer(this);
    
    connect(settingsCache, SIGNAL(soundPathChanged()), this, SLOT(cacheData()));
    connect(settingsCache, SIGNAL(soundEnabledChanged()), this, SLOT(soundEnabledChanged()));
    cacheData();
    soundEnabledChanged();

    lastTapPlayed = QDateTime::currentDateTime();
    lastEndStepPlayed = QDateTime::currentDateTime();
    lastStartCombatPlayed = QDateTime::currentDateTime();
}

void SoundEngine::cacheData()
{
    static const QStringList fileNames = QStringList()
        << "end_step" << "tap" << "player_joined" << "attack";
    for (int i = 0; i < fileNames.size(); ++i) {
        QFile file(settingsCache->getSoundPath() + "/" + fileNames[i] + ".raw");
        if(!file.exists())
            continue;
        file.open(QIODevice::ReadOnly);
        audioData.insert(fileNames[i], file.readAll());
        file.close();
    }
}

void SoundEngine::soundEnabledChanged()
{
    if (settingsCache->getSoundEnabled()) {
        qDebug("SoundEngine: enabling sound");
        QAudioFormat format;
#if QT_VERSION < 0x050000
        format.setFrequency(44100);
        format.setChannels(1);
#else
        format.setSampleRate(44100);
        format.setChannelCount(1);
#endif
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
}

void SoundEngine::playSound(const QString &fileName)
{
    if (!audio)
        return;
    
    audio->stop();
    inputBuffer->close();
    inputBuffer->setData(audioData[fileName]);
    inputBuffer->open(QIODevice::ReadOnly);
#if QT_VERSION >= 0x050000
    audio->setVolume(settingsCache->getMasterVolume() / 100.0);
#endif
    audio->start(inputBuffer);
    
}

void SoundEngine::endStep()
{
    if (lastEndStepPlayed.secsTo(QDateTime::currentDateTime()) >= 1)
        playSound("end_step");
    lastEndStepPlayed = QDateTime::currentDateTime();
}

void SoundEngine::tap()
{
    if (lastTapPlayed.secsTo(QDateTime::currentDateTime()) >= 1)
        playSound("tap");
    lastTapPlayed = QDateTime::currentDateTime();
}

void SoundEngine::playerJoined()
{
    playSound("player_joined");
}


void SoundEngine::attack() {
    if (lastStartCombatPlayed.secsTo(QDateTime::currentDateTime()) >= 1)
        playSound("attack");
    lastStartCombatPlayed = QDateTime::currentDateTime();
}
