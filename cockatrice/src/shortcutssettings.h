#ifndef SHORTCUTSSETTINGS_H
#define SHORTCUTSSETTINGS_H

#include <QApplication>
#include <QHash>
#include <QKeySequence>
#include <QObject>
#include <QSettings>

class ShortcutGroup
{
public:
    enum Groups
    {
        Main_Window,
        Deck_Editor,
        Game_Lobby,
        Card_Counters,
        Player_Counters,
        Power_Toughness,
        Game_Phases,
        Playing_Area,
        Move_selected,
        View,
        Move_top,
        Move_bottom,
        Gameplay,
        Drawing,
        Chat_room,
        Game_window,
        Load_deck
    };

    static QString getGroupName(ShortcutGroup::Groups group)
    {
        switch (group) {
            case Main_Window:
                return QApplication::translate("shortcutsTab", "Main Window");
            case Deck_Editor:
                return QApplication::translate("shortcutsTab", "Deck Editor");
            case Game_Lobby:
                return QApplication::translate("shortcutsTab", "Game Lobby");
            case Card_Counters:
                return QApplication::translate("shortcutsTab", "Card Counters");
            case Player_Counters:
                return QApplication::translate("shortcutsTab", "Player Counters");
            case Power_Toughness:
                return QApplication::translate("shortcutsTab", "Power and Toughness");
            case Game_Phases:
                return QApplication::translate("shortcutsTab", "Game Phases");
            case Playing_Area:
                return QApplication::translate("shortcutsTab", "Playing Area");
            case Move_selected:
                return QApplication::translate("shortcutsTab", "Move Selected Card");
            case View:
                return QApplication::translate("shortcutsTab", "View");
            case Move_top:
                return QApplication::translate("shortcutsTab", "Move Top Card");
            case Move_bottom:
                return QApplication::translate("shortcutsTab", "Move Bottom Card");
            case Gameplay:
                return QApplication::translate("shortcutsTab", "Gameplay");
            case Drawing:
                return QApplication::translate("shortcutsTab", "Drawing");
            case Chat_room:
                return QApplication::translate("shortcutsTab", "Chat Room");
            case Game_window:
                return QApplication::translate("shortcutsTab", "Game Window");
            case Load_deck:
                return QApplication::translate("shortcutsTab", "Load Deck from Clipboard");
        }

        return {};
    }
};

class ShortcutKey : public QList<QKeySequence>
{
public:
    ShortcutKey(const QString &_name = QString(),
                QList<QKeySequence> _sequence = QList<QKeySequence>(),
                ShortcutGroup::Groups _group = ShortcutGroup::Main_Window);
    void setSequence(QList<QKeySequence> _sequence)
    {
        QList<QKeySequence>::operator=(_sequence);
    };
    const QString getName() const
    {
        return QApplication::translate("shortcutsTab", name.toUtf8().data());
    };
    const QString getGroupName() const
    {
        return ShortcutGroup::getGroupName(group);
    };

private:
    QString name;
    ShortcutGroup::Groups group;
};

class ShortcutsSettings : public QObject
{
    Q_OBJECT
public:
    ShortcutsSettings(const QString &settingsFilePath, QObject *parent = nullptr);

    ShortcutKey getDefaultShortcut(const QString &name) const;
    ShortcutKey getShortcut(const QString &name) const;
    QKeySequence getSingleShortcut(const QString &name) const;
    QString getDefaultShortcutString(const QString &name) const;
    QString getShortcutString(const QString &name) const;
    QList<QString> getAllShortcutKeys() const
    {
        return shortCuts.keys();
    };

    void setShortcuts(const QString &name, const QList<QKeySequence> &Sequence);
    void setShortcuts(const QString &name, const QKeySequence &Sequence);
    void setShortcuts(const QString &name, const QString &Sequences);

    bool isKeyAllowed(const QString &name, const QString &Sequences) const;
    bool isValid(const QString &name, const QString &Sequences) const;

    void resetAllShortcuts();
    void clearAllShortcuts();

signals:
    void shortCutChanged();

private:
    const QChar sep = ';';
    const QString custom = "Custom"; // name of custom group in shortCutsFile
    QString settingsFilePath;
    QHash<QString, ShortcutKey> shortCuts;

    QString stringifySequence(const QList<QKeySequence> &Sequence) const;
    QList<QKeySequence> parseSequenceString(const QString &stringSequence) const;

    const QHash<QString, ShortcutKey> defaultShortCuts = {
        {"MainWindow/aCheckCardUpdates", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Check for Card Updates..."),
                                                     parseSequenceString(""),
                                                     ShortcutGroup::Main_Window)},
        {"MainWindow/aConnect", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Connect..."),
                                            parseSequenceString("Ctrl+L"),
                                            ShortcutGroup::Main_Window)},
        {"MainWindow/aDeckEditor", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Deck Editor"),
                                               parseSequenceString(""),
                                               ShortcutGroup::Main_Window)},
        {"MainWindow/aDisconnect", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Disconnect"),
                                               parseSequenceString(""),
                                               ShortcutGroup::Main_Window)},
        {"MainWindow/aExit",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Exit"), parseSequenceString(""), ShortcutGroup::Main_Window)},
        {"MainWindow/aFullScreen", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Full screen"),
                                               parseSequenceString("Ctrl+F"),
                                               ShortcutGroup::Main_Window)},
        {"MainWindow/aRegister", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Register..."),
                                             parseSequenceString(""),
                                             ShortcutGroup::Main_Window)},
        {"MainWindow/aSettings", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Settings..."),
                                             parseSequenceString("Ctrl+Shift+P"),
                                             ShortcutGroup::Main_Window)},
        {"MainWindow/aSinglePlayer", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Start a Local Game..."),
                                                 parseSequenceString("Ctrl+Shift+L"),
                                                 ShortcutGroup::Main_Window)},
        {"MainWindow/aWatchReplay", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Watch Replay..."),
                                                parseSequenceString(""),
                                                ShortcutGroup::Main_Window)},
        {"TabDeckEditor/aAnalyzeDeck", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Analyze Deck"),
                                                   parseSequenceString(""),
                                                   ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aClearFilterAll", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Clear All Filters"),
                                                      parseSequenceString(""),
                                                      ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aClearFilterOne", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Clear Selected Filter"),
                                                      parseSequenceString(""),
                                                      ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aClose",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Close"), parseSequenceString(""), ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aDecrement", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove Card"),
                                                 parseSequenceString("-"),
                                                 ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aManageSets", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Manage Sets..."),
                                                  parseSequenceString(""),
                                                  ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aEditTokens", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Edit Custom Tokens..."),
                                                  parseSequenceString(""),
                                                  ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aExportDeckDecklist", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Export Deck"),
                                                          parseSequenceString(""),
                                                          ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aIncrement", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add Card"),
                                                 parseSequenceString("+"),
                                                 ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aLoadDeck", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Load Deck..."),
                                                parseSequenceString("Ctrl+O"),
                                                ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aLoadDeckFromClipboard",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Load Deck from Clipboard..."),
                     parseSequenceString("Ctrl+Shift+V"),
                     ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aNewDeck", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "New Deck"),
                                               parseSequenceString("Ctrl+N"),
                                               ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aOpenCustomFolder",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Open Custom Pictures Folder"),
                     parseSequenceString(""),
                     ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aPrintDeck", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Print Deck..."),
                                                 parseSequenceString("Ctrl+P"),
                                                 ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aRemoveCard", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Delete Card"),
                                                  parseSequenceString(""),
                                                  ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aResetLayout", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Reset Layout"),
                                                   parseSequenceString(""),
                                                   ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aSaveDeck", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Save Deck"),
                                                parseSequenceString("Ctrl+S"),
                                                ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aSaveDeckAs", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Save Deck as..."),
                                                  parseSequenceString("Ctrl+Shift+S"),
                                                  ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aSaveDeckToClipboard",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Save Deck to Clipboard, Annotated"),
                     parseSequenceString("Ctrl+Shift+C"),
                     ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aSaveDeckToClipboardRaw",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Save Deck to Clipboard"),
                     parseSequenceString("Ctrl+Shift+R"),
                     ShortcutGroup::Deck_Editor)},
        {"DeckViewContainer/loadLocalButton", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Load Local Deck..."),
                                                          parseSequenceString("Ctrl+O"),
                                                          ShortcutGroup::Game_Lobby)},
        {"DeckViewContainer/loadRemoteButton", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Load Remote Deck..."),
                                                           parseSequenceString("Ctrl+Alt+O"),
                                                           ShortcutGroup::Game_Lobby)},
        {"DeckViewContainer/readyStartButton", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set Ready to Start"),
                                                           parseSequenceString("Ctrl+Shift+S"),
                                                           ShortcutGroup::Game_Lobby)},
        {"DeckViewContainer/sideboardLockButton",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Toggle Sideboard Lock"),
                     parseSequenceString("Ctrl+Shift+B"),
                     ShortcutGroup::Game_Lobby)},
        {"Player/aCCGreen", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add Green Counter"),
                                        parseSequenceString("Ctrl+>"),
                                        ShortcutGroup::Card_Counters)},
        {"Player/aRCGreen", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove Green Counter"),
                                        parseSequenceString("Ctrl+<"),
                                        ShortcutGroup::Card_Counters)},
        {"Player/aSCGreen", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set Green Counters..."),
                                        parseSequenceString("Ctrl+?"),
                                        ShortcutGroup::Card_Counters)},
        {"Player/aCCYellow", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add Yellow Counter"),
                                         parseSequenceString("Ctrl+."),
                                         ShortcutGroup::Card_Counters)},
        {"Player/aRCYellow", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove Yellow Counter"),
                                         parseSequenceString("Ctrl+,"),
                                         ShortcutGroup::Card_Counters)},
        {"Player/aSCYellow", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set Yellow Counters..."),
                                         parseSequenceString("Ctrl+/"),
                                         ShortcutGroup::Card_Counters)},
        {"Player/aCCRed", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add Red Counter"),
                                      parseSequenceString("Alt+."),
                                      ShortcutGroup::Card_Counters)},
        {"Player/aRCRed", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove Red Counter"),
                                      parseSequenceString("Alt+,"),
                                      ShortcutGroup::Card_Counters)},
        {"Player/aSCRed", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set Red Counters..."),
                                      parseSequenceString("Alt+/"),
                                      ShortcutGroup::Card_Counters)},
        {"Player/aInc", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add Life Counter"),
                                    parseSequenceString("F12"),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aDec", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove Life Counter"),
                                    parseSequenceString("F11"),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aSet", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set Life Counters..."),
                                    parseSequenceString("Ctrl+L"),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aIncCounter_w", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add White Counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aDecCounter_w", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove White Counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aSetCounter_w", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set White Counters..."),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aIncCounter_u", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add Blue Counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aDecCounter_u", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove Blue Counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aSetCounter_u", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set Blue Counters..."),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aIncCounter_b", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add Black Counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aDecCounter_b", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove Black Counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aSetCounter_b", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set Black Counters..."),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aIncCounter_r", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add Red Counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aDecCounter_r", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove Red Counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aSetCounter_r", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set Red Counters..."),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aIncCounter_g", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add Green Counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aDecCounter_g", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove Green Counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aSetCounter_g", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set Green Counters..."),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aIncCounter_x", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add Colorless Counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aDecCounter_x", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove Colorless Counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aSetCounter_x", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set Colorless Counters..."),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aIncCounter_storm", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add Other Counter"),
                                                 parseSequenceString("Ctrl+]"),
                                                 ShortcutGroup::Player_Counters)},
        {"Player/aDecCounter_storm", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove Other Counter"),
                                                 parseSequenceString("Ctrl+["),
                                                 ShortcutGroup::Player_Counters)},
        {"Player/aSetCounter_storm", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set Other Counters..."),
                                                 parseSequenceString("Ctrl+\\"),
                                                 ShortcutGroup::Player_Counters)},
        {"Player/aIncP", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add Power (+1/+0)"),
                                     parseSequenceString("Ctrl++;Ctrl+="),
                                     ShortcutGroup::Power_Toughness)},
        {"Player/aDecP", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove Power (-1/-0)"),
                                     parseSequenceString("Ctrl+-"),
                                     ShortcutGroup::Power_Toughness)},
        {"Player/aFlowP", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Move Toughness to Power (+1/-1)"),
                                      parseSequenceString(""),
                                      ShortcutGroup::Power_Toughness)},
        {"Player/aIncT", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add Toughness (+0/+1)"),
                                     parseSequenceString("Alt++;Alt+="),
                                     ShortcutGroup::Power_Toughness)},
        {"Player/aDecT", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove Toughness (-0/-1)"),
                                     parseSequenceString("Alt+-"),
                                     ShortcutGroup::Power_Toughness)},
        {"Player/aFlowT", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Move Power to Toughness (-1/+1)"),
                                      parseSequenceString(""),
                                      ShortcutGroup::Power_Toughness)},
        {"Player/aIncPT", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add Power and Toughness (+1/+1)"),
                                      parseSequenceString("Ctrl+Alt++;Ctrl+Alt+="),
                                      ShortcutGroup::Power_Toughness)},
        {"Player/aDecPT", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove Power and Toughness (-1/-1)"),
                                      parseSequenceString("Ctrl+Alt+-"),
                                      ShortcutGroup::Power_Toughness)},
        {"Player/aSetPT", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set Power and Toughness..."),
                                      parseSequenceString("Ctrl+P"),
                                      ShortcutGroup::Power_Toughness)},
        {"Player/aResetPT", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Reset Power and Toughness"),
                                        parseSequenceString("Ctrl+Alt+0"),
                                        ShortcutGroup::Power_Toughness)},
        {"Player/phase0", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Untap"),
                                      parseSequenceString("F5"),
                                      ShortcutGroup::Game_Phases)},
        {"Player/phase1",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Upkeep"), parseSequenceString(""), ShortcutGroup::Game_Phases)},
        {"Player/phase2",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Draw"), parseSequenceString("F6"), ShortcutGroup::Game_Phases)},
        {"Player/phase3", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "First Main Phase"),
                                      parseSequenceString("F7"),
                                      ShortcutGroup::Game_Phases)},
        {"Player/phase4", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Start Combat"),
                                      parseSequenceString("F8"),
                                      ShortcutGroup::Game_Phases)},
        {"Player/phase5",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Attack"), parseSequenceString(""), ShortcutGroup::Game_Phases)},
        {"Player/phase6",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Block"), parseSequenceString(""), ShortcutGroup::Game_Phases)},
        {"Player/phase7",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Damage"), parseSequenceString(""), ShortcutGroup::Game_Phases)},
        {"Player/phase8", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "End Combat"),
                                      parseSequenceString(""),
                                      ShortcutGroup::Game_Phases)},
        {"Player/phase9", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Second Main Phase"),
                                      parseSequenceString("F9"),
                                      ShortcutGroup::Game_Phases)},
        {"Player/phase10",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "End"), parseSequenceString("F10"), ShortcutGroup::Game_Phases)},
        {"Player/aNextPhase", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Next Phase"),
                                          parseSequenceString("Ctrl+Space;Tab"),
                                          ShortcutGroup::Game_Phases)},
        {"Player/aNextPhaseAction", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Next Phase Action"),
                                                parseSequenceString("Shift+Tab"),
                                                ShortcutGroup::Game_Phases)},
        {"Player/aNextTurn", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Next Turn"),
                                         parseSequenceString("Ctrl+Return;Ctrl+Enter"),
                                         ShortcutGroup::Game_Phases)},
        {"Player/aTap", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Tap / Untap Card"),
                                    parseSequenceString(""),
                                    ShortcutGroup::Playing_Area)},
        {"Player/aUntapAll", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Untap All"),
                                         parseSequenceString("Ctrl+U"),
                                         ShortcutGroup::Playing_Area)},
        {"Player/aDoesntUntap", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Toggle Untap"),
                                            parseSequenceString("Alt+U"),
                                            ShortcutGroup::Playing_Area)},
        {"Player/aFlip", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Turn Card Over"),
                                     parseSequenceString("Alt+F"),
                                     ShortcutGroup::Playing_Area)},
        {"Player/aPeek", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Peek Card"),
                                     parseSequenceString("Alt+L"),
                                     ShortcutGroup::Playing_Area)},
        {"Player/aPlay", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Play Card"),
                                     parseSequenceString(""),
                                     ShortcutGroup::Playing_Area)},
        {"Player/aAttach", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Attach Card..."),
                                       parseSequenceString("Ctrl+Alt+A"),
                                       ShortcutGroup::Playing_Area)},
        {"Player/aUnattach", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Unattach Card"),
                                         parseSequenceString("Ctrl+Alt+U"),
                                         ShortcutGroup::Playing_Area)},
        {"Player/aClone", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Clone Card"),
                                      parseSequenceString("Ctrl+J"),
                                      ShortcutGroup::Playing_Area)},
        {"Player/aCreateToken", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Create Token..."),
                                            parseSequenceString("Ctrl+T"),
                                            ShortcutGroup::Playing_Area)},
        {"Player/aCreateRelatedTokens", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Create All Related Tokens"),
                                                    parseSequenceString("Ctrl+Shift+T"),
                                                    ShortcutGroup::Playing_Area)},
        {"Player/aCreateAnotherToken", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Create Another Token"),
                                                   parseSequenceString("Ctrl+G"),
                                                   ShortcutGroup::Playing_Area)},
        {"Player/aSetAnnotation", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set Annotation..."),
                                              parseSequenceString("Alt+N"),
                                              ShortcutGroup::Playing_Area)},
        {"Player/aMoveToBottomLibrary", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Bottom of Library"),
                                                    parseSequenceString("Ctrl+B"),
                                                    ShortcutGroup::Move_selected)},
        {"Player/aMoveToExile", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Exile"),
                                            parseSequenceString(""),
                                            ShortcutGroup::Move_selected)},
        {"Player/aMoveToGraveyard", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Graveyard"),
                                                parseSequenceString("Ctrl+Del"),
                                                ShortcutGroup::Move_selected)},
        {"Player/aMoveToHand",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Hand"), parseSequenceString(""), ShortcutGroup::Move_selected)},
        {"Player/aMoveToTopLibrary", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Top of Library"),
                                                 parseSequenceString(""),
                                                 ShortcutGroup::Move_selected)},
        {"Player/aPlayFacedown", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Battlefield, Face Down"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Move_selected)},
        {"Player/aPlay", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Battlefield"),
                                     parseSequenceString(""),
                                     ShortcutGroup::Move_selected)},
        {"Player/aViewHand",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Hand"), parseSequenceString(""), ShortcutGroup::View)},
        {"Player/aViewGraveyard",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Graveyard"), parseSequenceString("F4"), ShortcutGroup::View)},
        {"Player/aViewLibrary",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Library"), parseSequenceString("F3"), ShortcutGroup::View)},
        {"Player/aViewRfg",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Exile"), parseSequenceString(""), ShortcutGroup::View)},
        {"Player/aViewSideboard", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Sideboard"),
                                              parseSequenceString("Ctrl+F3"),
                                              ShortcutGroup::View)},
        {"Player/aViewTopCards", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Top Cards of Library"),
                                             parseSequenceString("Ctrl+W"),
                                             ShortcutGroup::View)},
        {"Player/aCloseMostRecentZoneView", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Close Recent View"),
                                                        parseSequenceString("Esc"),
                                                        ShortcutGroup::View)},
        {"Player/aMoveTopToPlay", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Stack"),
                                              parseSequenceString("Ctrl+Y"),
                                              ShortcutGroup::Move_top)},
        {"Player/aMoveTopToPlayFaceDown", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Battlefield, Face Down"),
                                                      parseSequenceString("Ctrl+Shift+E"),
                                                      ShortcutGroup::Move_top)},
        {"Player/aMoveTopCardToGraveyard", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Graveyard"),
                                                       parseSequenceString("Alt+Y"),
                                                       ShortcutGroup::Move_top)},
        {"Player/aMoveTopCardsToGraveyard", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Graveyard (Multiple)"),
                                                        parseSequenceString("Alt+M"),
                                                        ShortcutGroup::Move_top)},
        {"Player/aMoveTopCardToExile",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Exile"), parseSequenceString(""), ShortcutGroup::Move_top)},
        {"Player/aMoveTopCardsToExile", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Exile (Multiple)"),
                                                    parseSequenceString(""),
                                                    ShortcutGroup::Move_top)},
        {"Player/aMoveTopCardsUntil", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Stack Until Found"),
                                                  parseSequenceString("Ctrl+Shift+Y"),
                                                  ShortcutGroup::Move_top)},
        {"Player/aMoveTopCardToBottom", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Bottom of Library"),
                                                    parseSequenceString(""),
                                                    ShortcutGroup::Move_top)},
        {"Player/aMoveBottomToPlay",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Stack"), parseSequenceString(""), ShortcutGroup::Move_bottom)},
        {"Player/aMoveBottomToPlayFaceDown", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Battlefield, Face Down"),
                                                         parseSequenceString(""),
                                                         ShortcutGroup::Move_bottom)},
        {"Player/aMoveBottomCardToGrave", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Graveyard"),
                                                      parseSequenceString(""),
                                                      ShortcutGroup::Move_bottom)},
        {"Player/aMoveBottomCardsToGrave", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Graveyard (Multiple)"),
                                                       parseSequenceString(""),
                                                       ShortcutGroup::Move_bottom)},
        {"Player/aMoveBottomCardToExile",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Exile"), parseSequenceString(""), ShortcutGroup::Move_bottom)},
        {"Player/aMoveBottomCardsToExile", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Exile (Multiple)"),
                                                       parseSequenceString(""),
                                                       ShortcutGroup::Move_bottom)},
        {"Player/aMoveBottomCardToTop", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Top of Library"),
                                                    parseSequenceString(""),
                                                    ShortcutGroup::Move_bottom)},
        {"Player/aDrawBottomCard", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Draw Bottom Card"),
                                               parseSequenceString(""),
                                               ShortcutGroup::Move_bottom)},
        {"Player/aDrawBottomCards", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Draw Multiple Cards from Bottom..."),
                                                parseSequenceString(""),
                                                ShortcutGroup::Move_bottom)},
        {"Player/aDrawArrow", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Draw Arrow..."),
                                          parseSequenceString("Ctrl+A"),
                                          ShortcutGroup::Gameplay)},
        {"Player/aRemoveLocalArrows", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove Local Arrows"),
                                                  parseSequenceString("Ctrl+R"),
                                                  ShortcutGroup::Gameplay)},
        {"Player/aLeaveGame", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Leave Game"),
                                          parseSequenceString("Ctrl+Q"),
                                          ShortcutGroup::Gameplay)},
        {"Player/aConcede",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Concede"), parseSequenceString("F2"), ShortcutGroup::Gameplay)},
        {"Player/aRollDie", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Roll Dice..."),
                                        parseSequenceString("Ctrl+I"),
                                        ShortcutGroup::Gameplay)},
        {"Player/aShuffle", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Shuffle Library"),
                                        parseSequenceString("Ctrl+S"),
                                        ShortcutGroup::Gameplay)},
        {"Player/aMulligan", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Mulligan"),
                                         parseSequenceString("Ctrl+M"),
                                         ShortcutGroup::Drawing)},
        {"Player/aDrawCard", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Draw a Card"),
                                         parseSequenceString("Ctrl+D"),
                                         ShortcutGroup::Drawing)},
        {"Player/aDrawCards", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Draw Multiple Cards..."),
                                          parseSequenceString("Ctrl+E"),
                                          ShortcutGroup::Drawing)},
        {"Player/aUndoDraw", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Undo Draw"),
                                         parseSequenceString("Ctrl+Shift+D"),
                                         ShortcutGroup::Drawing)},
        {"Player/aAlwaysRevealTopCard", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Always Reveal Top Card"),
                                                    parseSequenceString("Ctrl+N"),
                                                    ShortcutGroup::Drawing)},
        {"Player/aAlwaysLookAtTopCard", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Always Look At Top Card"),
                                                    parseSequenceString("Ctrl+Shift+N"),
                                                    ShortcutGroup::Drawing)},
        {"Player/aRotateViewCW", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Rotate View Clockwise"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Gameplay)},
        {"Player/aRotateViewCCW", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Rotate View Counterclockwise"),
                                              parseSequenceString(""),
                                              ShortcutGroup::Gameplay)},
        {"Textbox/unfocusTextBox", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Unfocus Text Box"),
                                               parseSequenceString("Esc"),
                                               ShortcutGroup::Chat_room)},
        {"tab_game/aFocusChat", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Focus Chat"),
                                            parseSequenceString("Shift+Return"),
                                            ShortcutGroup::Chat_room)},
        {"tab_room/aClearChat", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Clear Chat"),
                                            parseSequenceString("F12"),
                                            ShortcutGroup::Chat_room)},
        {"Player/aResetLayout", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Reset Layout"),
                                            parseSequenceString(""),
                                            ShortcutGroup::Game_window)},
        {"DlgLoadDeckFromClipboard/refreshButton", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Refresh"),
                                                               parseSequenceString("F5"),
                                                               ShortcutGroup::Load_deck)}};
};

#endif // SHORTCUTSSETTINGS_H
