#include "shortcuts_settings.h"

#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QStringList>
#include <utility>

ShortcutKey::ShortcutKey(const QString &_name, QList<QKeySequence> _sequence, ShortcutGroup::Groups _group)
    : QList<QKeySequence>(_sequence), name(_name), group(_group)
{
}

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

            // check whether shortcut name exists
            if (!shortCuts.contains(*it)) {
                qCWarning(ShortcutsSettingsLog) << "Unknown shortcut name:" << *it;
                continue;
            }

            // check whether shortcut is forbidden
            if (isKeyAllowed(*it, stringSequence)) {
                auto shortcut = getShortcut(*it);
                shortcut.setSequence(parseSequenceString(stringSequence));
                shortCuts.insert(*it, shortcut);
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
        }
    }
}

/// PR 5079 changes Textbox/unfocusTextBox to Player/unfocusTextBox and tab_game/aFocusChat to Player/aFocusChat.
/// A migration is necessary to let players keep their already configured shortcuts.
void ShortcutsSettings::migrateShortcuts()
{
    if (QFile(settingsFilePath).exists()) {
        QSettings shortCutsFile(settingsFilePath, QSettings::IniFormat);

        shortCutsFile.beginGroup(custom);

        if (shortCutsFile.contains("Textbox/unfocusTextBox")) {
            qCInfo(ShortcutsSettingsLog)
                << "[ShortcutsSettings] Textbox/unfocusTextBox shortcut found. Migrating to Player/unfocusTextBox.";
            QString unfocusTextBox = shortCutsFile.value("Textbox/unfocusTextBox", "").toString();
            this->setShortcuts("Player/unfocusTextBox", unfocusTextBox);
            shortCutsFile.remove("Textbox/unfocusTextBox");
        }

        if (shortCutsFile.contains("tab_game/aFocusChat")) {
            qCInfo(ShortcutsSettingsLog)
                << "[ShortcutsSettings] tab_game/aFocusChat shortcut found. Migrating to Player/aFocusChat.";
            QString aFocusChat = shortCutsFile.value("tab_game/aFocusChat", "").toString();
            this->setShortcuts("Player/aFocusChat", aFocusChat);
            shortCutsFile.remove("tab_game/aFocusChat");
        }

        // PR #5564 changes "MainWindow/aDeckEditor" to "Tabs/aTabDeckEditor"
        if (shortCutsFile.contains("MainWindow/aDeckEditor")) {
            qCInfo(ShortcutsSettingsLog) << "MainWindow/aDeckEditor shortcut found. Migrating to Tabs/aTabDeckEditor.";
            QString keySequence = shortCutsFile.value("MainWindow/aDeckEditor", "").toString();
            this->setShortcuts("Tabs/aTabDeckEditor", keySequence);
            shortCutsFile.remove("MainWindow/aDeckEditor");
        }

        shortCutsFile.endGroup();
    }
}

ShortcutKey ShortcutsSettings::getDefaultShortcut(const QString &name) const
{
    return defaultShortCuts.value(name, ShortcutKey());
}

ShortcutKey ShortcutsSettings::getShortcut(const QString &name) const
{
    if (shortCuts.contains(name)) {
        return shortCuts.value(name);
    }

    return getDefaultShortcut(name);
}

/**
 * Gets the first shortcut for the given action.
 *
 * NOTE: In most cases you should be using ShortcutsSettings::getShortcut instead,
 * as that will return all shortcuts if there are multiple shortcuts.
 * The only reason to use this method is if an object does not accept multiple shortcuts, such as with QButtons.
 */
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

QString ShortcutsSettings::getShortcutFriendlyName(const QString &shortcutName) const
{
    for (auto it = defaultShortCuts.cbegin(); it != defaultShortCuts.cend(); ++it) {
        if (shortcutName == it.key()) {
            return it.value().getName();
        }
    }

    return {};
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
    shortCuts[name].setSequence(Sequence);

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

void ShortcutsSettings::setShortcuts(const QString &name, const QString &sequences)
{
    setShortcuts(name, parseSequenceString(sequences));
}

void ShortcutsSettings::resetAllShortcuts()
{
    shortCuts = defaultShortCuts;
    QSettings shortCutsFile(settingsFilePath, QSettings::IniFormat);
    shortCutsFile.beginGroup(custom);
    shortCutsFile.remove("");
    shortCutsFile.endGroup();
    emit shortCutChanged();
}

void ShortcutsSettings::clearAllShortcuts()
{
    QSettings shortCutsFile(settingsFilePath, QSettings::IniFormat);
    shortCutsFile.beginGroup(custom);
    for (auto it = shortCuts.begin(); it != shortCuts.end(); ++it) {
        it.value().setSequence(parseSequenceString(""));
        shortCutsFile.setValue(it.key(), "");
    }
    shortCutsFile.endGroup();
    emit shortCutChanged();
}

bool ShortcutsSettings::isKeyAllowed(const QString &name, const QString &sequences) const
{
    // if the shortcut is not to be used in deck-editor then it doesn't matter
    if (name.startsWith("Player") || name.startsWith("Replays")) {
        return true;
    }
    QString checkSequence = sequences.split(sep).last();
    QStringList forbiddenKeys{"Del",        "Backspace", "Down",  "Up",         "Left",       "Right",
                              "Return",     "Enter",     "Menu",  "Ctrl+Alt+-", "Ctrl+Alt+=", "Ctrl+Alt+[",
                              "Ctrl+Alt+]", "Tab",       "Space", "Shift+S",    "Shift+Left", "Shift+Right"};
    return !forbiddenKeys.contains(checkSequence);
}

/**
 *  Checks that the shortcut doesn't overlap with an existing shortcut
 *
 * @param name The name of the shortcut
 * @param sequences The shortcut key sequence
 * @return Whether the shortcut is valid.
 */
bool ShortcutsSettings::isValid(const QString &name, const QString &sequences) const
{
    return findOverlaps(name, sequences).isEmpty();
}

/**
 * Checks if the shortcut is a shortcut that is active in all windows
 */
static bool isAlwaysActiveShortcut(const QString &shortcutName)
{
    return shortcutName.startsWith("MainWindow") || shortcutName.startsWith("Tabs");
}

QStringList ShortcutsSettings::findOverlaps(const QString &name, const QString &sequences) const
{
    QString checkSequence = sequences.split(sep).last();
    QString checkKey = name.left(name.indexOf("/"));

    QStringList overlaps;
    for (const auto &key : shortCuts.keys()) {
        if (key.startsWith(checkKey) || isAlwaysActiveShortcut(key) || isAlwaysActiveShortcut(checkKey)) {
            QString storedSequence = stringifySequence(shortCuts.value(key));
            if (storedSequence.split(sep).contains(checkSequence)) {
                overlaps.append(getShortcutFriendlyName(key));
            }
        }
    }

    return overlaps;
}