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
}

void SoundEngine::cacheData()
{
    static const QStringList fileNames = QStringList()
        << "notification" << "draw" << "playcard" << "shuffle" << "tap" << "untap" << "cuckoo";
    for (int i = 0; i < fileNames.size(); ++i) {
        QFile file(settingsCache->getSoundPath() + "/" + fileNames[i] + ".raw");
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
        format.setFrequency(44100);
        format.setChannels(1);
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
    audio->start(inputBuffer);
}

void SoundEngine::notification()
{
    playSound("notification");
}

void SoundEngine::draw()
{
    playSound("draw");
}

void SoundEngine::playCard()
{
    playSound("playcard");
}

void SoundEngine::shuffle()
{
    playSound("shuffle");
}

void SoundEngine::tap()
{
    playSound("tap");
}

void SoundEngine::untap()
{
    playSound("untap");
}

void SoundEngine::cuckoo()
{
    playSound("cuckoo");
}
