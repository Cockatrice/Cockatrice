#ifndef SOUNDENGINE_H
#define SOUNDENGINE_H

#include <QObject>
#include <QMap>

class QAudioOutput;
class QBuffer;

class SoundEngine : public QObject {
    Q_OBJECT
private:
    void playSound(const QString &fileName);
    QMap<QString, QByteArray> audioData;
    QBuffer *inputBuffer;
    QAudioOutput *audio;
private slots:
    void cacheData();
    void soundEnabledChanged();
public:
    SoundEngine(QObject *parent = 0);
public slots:
    void notification();
    void draw();
    void playCard();
    void shuffle();
    void tap();
    void untap();
    void cuckoo();
};

extern SoundEngine *soundEngine;

#endif
