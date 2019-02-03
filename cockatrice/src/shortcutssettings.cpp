#include "shortcutssettings.h"
#include <QFile>
#include <QMessageBox>
#include <QStringList>
#include <utility>

ShortcutsSettings::ShortcutsSettings(const QString &settingsPath, QObject *parent) : QObject(parent)
{
    shortCuts = defaultShortCuts;
    settingsFilePath = settingsPath;
    settingsFilePath.append("shortcuts.ini");

    bool exists = QFile(settingsFilePath).exists();

    QSettings shortCutsFile(settingsFilePath, QSettings::IniFormat);

    if (exists) {
        shortCutsFile.beginGroup(custom);
        const QStringList customKeys = shortCutsFile.allKeys();

        QMap<QString, QString> invalidItems;
        for (QStringList::const_iterator it = customKeys.constBegin(); it != customKeys.constEnd(); ++it) {
            QString stringSequence = shortCutsFile.value(*it).toString();
            // check whether shortcut is forbidden
            if (isKeyAllowed(*it, stringSequence)) {
                QList<QKeySequence> SequenceList = parseSequenceString(stringSequence);
                shortCuts.insert(*it, SequenceList);
            } else {
                invalidItems.insert(*it, stringSequence);
            }
        }

        shortCutsFile.endGroup();

        if (!invalidItems.isEmpty()) {
            // warning message in case of invalid items
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Your configuration file contained invalid shortcuts.\n"
                              "Please check your shortcut settings!"));
            QString detailedMessage = tr("The following shortcuts have been set to default:\n");
            for (QMap<QString, QString>::const_iterator item = invalidItems.constBegin();
                 item != invalidItems.constEnd(); ++item) {
                detailedMessage += item.key() + " - \"" + item.value() + "\"\n";
            }
            msgBox.setDetailedText(detailedMessage);
            msgBox.exec();

            // set default shortcut where stored value was invalid
            for (const QString &key : invalidItems.keys()) {
                setShortcuts(key, getDefaultShortcutString(key));
            }
        }
    }
}

QList<QKeySequence> ShortcutsSettings::getDefaultShortcut(const QString &name) const
{
    return defaultShortCuts.value(name, QList<QKeySequence>());
}

QList<QKeySequence> ShortcutsSettings::getShortcut(const QString &name) const
{
    if (shortCuts.contains(name)) {
        return shortCuts.value(name);
    }

    return getDefaultShortcut(name);
}

QKeySequence ShortcutsSettings::getSingleShortcut(const QString &name) const
{
    return getShortcut(name).at(0);
}

QString ShortcutsSettings::getDefaultShortcutString(const QString &name) const
{
    return stringifySequence(getDefaultShortcut(name));
}

QString ShortcutsSettings::getShortcutString(const QString &name) const
{
    return stringifySequence(getShortcut(name));
}

QString ShortcutsSettings::stringifySequence(const QList<QKeySequence> &Sequence) const
{
    QStringList stringSequence;
    for (const auto &i : Sequence) {
        stringSequence.append(i.toString(QKeySequence::PortableText));
    }

    return stringSequence.join(sep);
}

QList<QKeySequence> ShortcutsSettings::parseSequenceString(const QString &stringSequence) const
{
    QList<QKeySequence> SequenceList;
    for (const QString &shortcut : stringSequence.split(sep)) {
        SequenceList.append(QKeySequence(shortcut, QKeySequence::PortableText));
    }

    return SequenceList;
}

void ShortcutsSettings::setShortcuts(const QString &name, const QList<QKeySequence> &Sequence)
{
    shortCuts[name] = Sequence;

    QSettings shortCutsFile(settingsFilePath, QSettings::IniFormat);
    shortCutsFile.beginGroup(custom);
    shortCutsFile.setValue(name, stringifySequence(Sequence));
    shortCutsFile.endGroup();
    emit shortCutChanged();
}

void ShortcutsSettings::setShortcuts(const QString &name, const QKeySequence &Sequence)
{
    setShortcuts(name, QList<QKeySequence>{Sequence});
}

void ShortcutsSettings::setShortcuts(const QString &name, const QString &Sequences)
{
    setShortcuts(name, parseSequenceString(Sequences));
}

void ShortcutsSettings::resetAllShortcuts()
{
    shortCuts = defaultShortCuts;
    QSettings shortCutsFile(settingsFilePath, QSettings::IniFormat);
    shortCutsFile.beginGroup(custom);
    shortCutsFile.remove("");
    shortCutsFile.endGroup();
    emit shortCutChanged();
    emit allShortCutsReset();
}

void ShortcutsSettings::clearAllShortcuts()
{
    QSettings shortCutsFile(settingsFilePath, QSettings::IniFormat);
    shortCutsFile.beginGroup(custom);
    for (auto it = shortCuts.begin(); it != shortCuts.end(); ++it) {
        it.value() = parseSequenceString("");
        shortCutsFile.setValue(it.key(), "");
    }
    shortCutsFile.endGroup();
    emit shortCutChanged();
    emit allShortCutsClear();
}

bool ShortcutsSettings::isKeyAllowed(const QString &name, const QString &Sequences) const
{
    // if the shortcut is not to be used in deck-editor then it doesn't matter
    if (name.startsWith("Player")) {
        return true;
    }
    QString checkSequence = Sequences.split(sep).last();
    QStringList forbiddenKeys{"Del",        "Backspace", "Down",  "Up",         "Left",       "Right",
                              "Return",     "Enter",     "Menu",  "Ctrl+Alt+-", "Ctrl+Alt+=", "Ctrl+Alt+[",
                              "Ctrl+Alt+]", "Tab",       "Space", "Shift+S",    "Shift+Left", "Shift+Right"};
    return !forbiddenKeys.contains(checkSequence);
}

bool ShortcutsSettings::isValid(const QString &name, const QString &Sequences) const
{
    QString checkSequence = Sequences.split(sep).last();
    QString checkKey = name.left(name.indexOf("/"));

    QList<QString> allKeys = shortCuts.keys();
    for (const auto &key : allKeys) {
        if (key.startsWith(checkKey) || key.startsWith("MainWindow") || checkKey.startsWith("MainWindow")) {
            QString storedSequence = stringifySequence(shortCuts.value(key));
            QStringList stringSequences = storedSequence.split(sep);
            if (stringSequences.contains(checkSequence)) {
                return false;
            }
        }
    }
    return true;
}
