#ifndef SOUNDENGINE_H
#define SOUNDENGINE_H

#include <QObject>

class SoundEngine : public QObject {
    Q_OBJECT
private:
    bool enabled;
private slots:
    void soundEnabledChanged();
public:
    SoundEngine(QObject *parent = 0);
    void playSound(QString fileName);
public slots:
    void testSound();
};

extern SoundEngine *soundEngine;
#endif
