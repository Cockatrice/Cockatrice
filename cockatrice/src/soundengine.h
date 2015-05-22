#ifndef SOUNDENGINE_H
#define SOUNDENGINE_H

#include <QObject>
#include <QMap>
#include <QDateTime>

class QAudioOutput;
class QBuffer;

class SoundEngine : public QObject {
    Q_OBJECT
private:
    void playSound(const QString &fileName);
    QMap<QString, QByteArray> audioData;
    QBuffer *inputBuffer;
    QAudioOutput *audio;
    QDateTime lastTapPlayed;
    QDateTime lastEndStepPlayed;
    QDateTime lastAttackStepPlayed;
private slots:
    void cacheData();
    void soundEnabledChanged();
public:
    SoundEngine(QObject *parent = 0);
public slots:
    void endStep();
    void tap();
    void playerJoined();
    void attack();
};

extern SoundEngine *soundEngine;

#endif
