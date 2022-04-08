#ifndef SOUNDENGINE_H
#define SOUNDENGINE_H

#include <QDir>
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
    QBuffer *inputBuffer;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QMap<QString, QString> audioData;
    QMediaPlayer *player;
#else
    QMap<QString, QByteArray> audioData;
    QAudioOutput *player;
#endif

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
