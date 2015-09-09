#ifndef SOUNDENGINE_H
#define SOUNDENGINE_H

#include <QObject>
#include <QMap>
#include <QDateTime>
#include <QStringList>
#if QT_VERSION > 0x050000
    #include <QMediaPlayer>
#endif

class QAudioOutput;
class QBuffer;

class SoundEngine : public QObject {
    Q_OBJECT
private:
    QMap<QString, QByteArray> audioCache;
    QBuffer *inputBuffer;
    QAudioOutput *audio;
    bool enabled;
    QStringList fileNames;
    
	#if QT_VERSION > 0x050000
        QMap<QString, QMediaPlayer*> audioData;
    #endif
private slots:
    void cacheData();
    void soundEnabledChanged();
public:
    SoundEngine(QObject *parent = 0);
    void playSound(QString fileName);
public slots:
    void testSound();
};

extern SoundEngine *soundEngine;
#endif
