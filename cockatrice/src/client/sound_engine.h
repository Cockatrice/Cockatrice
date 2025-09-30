/**
 * @file sound_engine.h
 * @ingroup Core
 * @brief TODO: Document this.
 */

#ifndef SOUNDENGINE_H
#define SOUNDENGINE_H

#include <QAudioOutput>
#include <QLoggingCategory>
#include <QMap>
#include <QMediaPlayer>
#include <QObject>
#include <QString>

inline Q_LOGGING_CATEGORY(SoundEngineLog, "sound_engine");

class QBuffer;

typedef QMap<QString, QString> QStringMap;

class SoundEngine : public QObject
{
    Q_OBJECT
public:
    explicit SoundEngine(QObject *parent = nullptr);
    ~SoundEngine() override;
    void playSound(const QString &fileName);
    QStringMap &getAvailableThemes();

private:
    QStringMap availableThemes;
    QMap<QString, QString> audioData;
    QAudioOutput *audioOutput;
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
