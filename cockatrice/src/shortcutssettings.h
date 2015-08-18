#ifndef SHORTCUTSSETTINGS_H
#define SHORTCUTSSETTINGS_H

#include <QObject>
#include <QSettings>
#include <QMap>
#include <QKeySequence>

class ShortcutsSettings : public QObject
{
    Q_OBJECT
public:
    ShortcutsSettings(QString settingsFilePath, QObject *parent = 0);
    ~ShortcutsSettings() { }

    QList<QKeySequence> getShortcut(QString name);
    QKeySequence getSingleShortcut(QString name);

    QString getDefaultShortcutString(QString name);
    QString getShortcutString(QString name);

    void setShortcuts(QString name, QList<QKeySequence> secuence);
    void setShortcuts(QString name, QKeySequence secuence);
    void setShortcuts(QString name, QString secuences);

    bool isValid(QString name, QString secuences);
signals:
    void shortCutchanged();

private:
    QString settingsFilePath;
    QMap<QString,QList<QKeySequence> > shortCuts;
    QMap<QString,QList<QKeySequence> > defaultShortCuts;
    void fillDefaultShorcuts();

    QString stringifySecuence(QList<QKeySequence> secuence) const;
    QList<QKeySequence> parseSecuenceString(QString stringSecuence);
};

#endif // SHORTCUTSSETTINGS_H
