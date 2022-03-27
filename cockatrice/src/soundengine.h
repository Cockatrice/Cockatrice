#ifndef SOUNDENGINE_H
#define SOUNDENGINE_H

#include <QMap>
#include <QMediaPlayer>
#include <QObject>
#include <QString>

class QAudioOutput;
class QBuffer;

typedef QMap<QString, QString> QStringMap;

class SoundEngine : public QObject
{
    Q_OBJECT
public:
    SoundEngine(QObject *parent = nullptr);
    ~SoundEngine();
    void playSound(QString fileName);
    QStringMap &getAvailableThemes();

private:
    QStringMap availableThemes;
    QMap<QString, QString> audioData;
    QMediaPlayer *player;

protected:
    void ensureThemeDirectoryExists();
private slots:
    void soundEnabledChanged();
    void themeChangedSlot();
public slots:
    void testSound();
};

extern SoundEngine *soundEngine;
#endif
