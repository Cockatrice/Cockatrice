/**
 * @file sound_engine.h
 * @ingroup Core
 */
//! \todo Document this file.

#ifndef SOUNDENGINE_H
#define SOUNDENGINE_H

#include <QLoggingCategory>
#include <QMap>
#include <QObject>
#include <QString>
#include <qtmetamacros.h>

class QAudioOutput;
class QMediaPlayer;

inline Q_LOGGING_CATEGORY(SoundEngineLog, "sound_engine");

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
