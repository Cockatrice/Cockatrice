#ifndef SOUNDENGINE_H
#define SOUNDENGINE_H

#include <QObject>
#include <QMap>
#include <QDir>
#include <QString>

typedef QMap<QString, QString> QStringMap;

class SoundEngine : public QObject {
    Q_OBJECT
public:
    SoundEngine(QObject *parent = 0);
    void playSound(QString fileName);
    QStringMap &getAvailableThemes();
private:
    bool enabled;
    QStringMap availableThemes;
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
