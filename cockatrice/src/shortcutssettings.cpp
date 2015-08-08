#include "shortcutssettings.h"

#include <QDebug>
QList<QKeySequence> ShortcutsSettings::parseSecuenceString(QString stringSecuence)
{
    QStringList secuences = stringSecuence.split(";");
    QList<QKeySequence> secuenceList;
    for(QStringList::const_iterator ss = secuences.cbegin(); ss != secuences.cend(); ++ss)
    {
        secuenceList.append(QKeySequence(*ss, QKeySequence::PortableText));
    }

    return secuenceList;
}

ShortcutsSettings::ShortcutsSettings(QString settingsPath, QObject *parent) : QObject(parent)
{
    this->settingsFilePath = settingsPath;
    this->settingsFilePath.append("shortcuts.ini");
    QSettings shortCutsFile(settingsFilePath, QSettings::IniFormat);

    shortCutsFile.beginGroup("Custom");
    const QStringList customKeys = shortCutsFile.allKeys();
    for(QStringList::const_iterator it = customKeys.cbegin(); it != customKeys.cend(); ++it)
    {
        QString stringSecuence = shortCutsFile.value(*it).toString();
        QList<QKeySequence> secuenceList = parseSecuenceString(stringSecuence);
        shortCuts.insert(*it, secuenceList);
    }
    shortCutsFile.endGroup();

    shortCutsFile.beginGroup("Defaults");
    const QStringList defaultKeys = shortCutsFile.allKeys();
    for(QStringList::const_iterator it = defaultKeys.cbegin(); it != defaultKeys.cend(); ++it)
    {
        QString stringSecuence = shortCutsFile.value(*it).toString();
        QList<QKeySequence> secuenceList = parseSecuenceString(stringSecuence);
        defaultShortCuts.insert(*it, secuenceList);
    }
    shortCutsFile.endGroup();
}

QList<QKeySequence> ShortcutsSettings::getShortcut(QString name, QList<QKeySequence> defaultShortCut)
{
    if(shortCuts.contains(name))
        return shortCuts.value(name);

    setShortcuts(name, defaultShortCut);

    defaultShortCuts[name] = defaultShortCut;

    QSettings shortCutsFile(settingsFilePath, QSettings::IniFormat);
    shortCutsFile.beginGroup("Defaults");
    shortCutsFile.setValue(name, stringifySecuence(defaultShortCut));
    shortCutsFile.endGroup();
    return defaultShortCut;
}

QList<QKeySequence> ShortcutsSettings::getShortcut(QString name, QKeySequence defaultShortCut)
{
    return getShortcut(name, QList<QKeySequence>() << defaultShortCut);
}

QKeySequence ShortcutsSettings::getSingleShortcut(QString name, QKeySequence defaultShortCut)
{
    return getShortcut(name,defaultShortCut).at(0);
}

QList<QKeySequence> ShortcutsSettings::getDefaultShortCut(QString name)
{
    return defaultShortCuts.value(name);
}

QString ShortcutsSettings::stringifySecuence(QList<QKeySequence> secuence) const
{
    QString stringSecuence;
    for(int i=0; i < secuence.size(); ++i)
    {
        stringSecuence.append(secuence.at(i).toString(QKeySequence::PortableText));
        if(i < secuence.size() - 1)
            stringSecuence.append(";");
    }

    return stringSecuence;
}

void ShortcutsSettings::setShortcuts(QString name, QList<QKeySequence> secuence)
{
    shortCuts[name] = secuence;

    QSettings shortCutsFile(settingsFilePath, QSettings::IniFormat);
    shortCutsFile.beginGroup("Custom");
    QString stringSecuence = stringifySecuence(secuence);
    shortCutsFile.setValue(name, stringSecuence);
    shortCutsFile.endGroup();
}

void ShortcutsSettings::setShortcuts(QString name, QKeySequence secuence)
{
    setShortcuts(name, QList<QKeySequence>() << secuence);
}

