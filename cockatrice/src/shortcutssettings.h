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
                return QApplication::translate("shortcutsTab", "Card counters");
            case Player_Counters:
                return QApplication::translate("shortcutsTab", "Player counters");
            case Power_Toughness:
                return QApplication::translate("shortcutsTab", "Power and Toughness");
            case Game_Phases:
                return QApplication::translate("shortcutsTab", "Game Phases");
            case Playing_Area:
                return QApplication::translate("shortcutsTab", "Playing Area");
            case Move_selected:
                return QApplication::translate("shortcutsTab", "Move selected card to");
            case View:
                return QApplication::translate("shortcutsTab", "View");
            case Move_top:
                return QApplication::translate("shortcutsTab", "Move top card to");
            case Gameplay:
                return QApplication::translate("shortcutsTab", "Gameplay");
            case Drawing:
                return QApplication::translate("shortcutsTab", "Drawing");
            case Chat_room:
                return QApplication::translate("shortcutsTab", "Chat room");
            case Game_window:
                return QApplication::translate("shortcutsTab", "Game window");
            case Load_deck:
                return QApplication::translate("shortcutsTab", "Load deck from clipboard");
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
        {"MainWindow/aCheckCardUpdates", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Check for card updates"),
                                                     parseSequenceString(""),
                                                     ShortcutGroup::Main_Window)},
        {"MainWindow/aConnect", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Connect"),
                                            parseSequenceString("Ctrl+L"),
                                            ShortcutGroup::Main_Window)},
        {"MainWindow/aDeckEditor", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Deck editor"),
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
        {"MainWindow/aRegister", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Register"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Main_Window)},
        {"MainWindow/aSettings", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Settings"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Main_Window)},
        {"MainWindow/aSinglePlayer", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Start local game"),
                                                 parseSequenceString(""),
                                                 ShortcutGroup::Main_Window)},
        {"MainWindow/aWatchReplay", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Watch replay"),
                                                parseSequenceString(""),
                                                ShortcutGroup::Main_Window)},
        {"TabDeckEditor/aAnalyzeDeck", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Analyze deck"),
                                                   parseSequenceString(""),
                                                   ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aClearFilterAll", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Clear all filters"),
                                                      parseSequenceString(""),
                                                      ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aClearFilterOne", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Clear selected filter"),
                                                      parseSequenceString(""),
                                                      ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aClose",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Close"), parseSequenceString(""), ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aDecrement", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove card"),
                                                 parseSequenceString("-"),
                                                 ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aManageSets", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Manage sets"),
                                                  parseSequenceString(""),
                                                  ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aEditTokens", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Edit custom tokens"),
                                                  parseSequenceString(""),
                                                  ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aExportDeckDecklist", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Export deck"),
                                                          parseSequenceString(""),
                                                          ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aIncrement", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add card"),
                                                 parseSequenceString("+"),
                                                 ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aLoadDeck", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Load deck"),
                                                parseSequenceString("Ctrl+O"),
                                                ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aLoadDeckFromClipboard", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Load deck (clipboard)"),
                                                             parseSequenceString("Ctrl+Shift+V"),
                                                             ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aNewDeck", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "New deck"),
                                               parseSequenceString("Ctrl+N"),
                                               ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aOpenCustomFolder", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Open custom pic folder"),
                                                        parseSequenceString(""),
                                                        ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aPrintDeck", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Print deck"),
                                                 parseSequenceString("Ctrl+P"),
                                                 ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aRemoveCard", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Delete card"),
                                                  parseSequenceString(""),
                                                  ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aResetLayout", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Reset layout"),
                                                   parseSequenceString(""),
                                                   ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aSaveDeck", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Save deck"),
                                                parseSequenceString("Ctrl+S"),
                                                ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aSaveDeckAs", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Save deck as"),
                                                  parseSequenceString(""),
                                                  ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aSaveDeckToClipboard",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Save deck to clipboard annotated"),
                     parseSequenceString("Ctrl+Shift+C"),
                     ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aSaveDeckToClipboardRaw",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Save deck to clipboard raw"),
                     parseSequenceString("Ctrl+Shift+R"),
                     ShortcutGroup::Deck_Editor)},
        {"DeckViewContainer/loadLocalButton", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Load local deck"),
                                                          parseSequenceString("Ctrl+O"),
                                                          ShortcutGroup::Game_Lobby)},
        {"DeckViewContainer/loadRemoteButton", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Load remote deck"),
                                                           parseSequenceString("Ctrl+Alt+O"),
                                                           ShortcutGroup::Game_Lobby)},
        {"Player/aCCGreen", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add green counter"),
                                        parseSequenceString(""),
                                        ShortcutGroup::Card_Counters)},
        {"Player/aRCGreen", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove green counter"),
                                        parseSequenceString(""),
                                        ShortcutGroup::Card_Counters)},
        {"Player/aSCGreen", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set green counters"),
                                        parseSequenceString(""),
                                        ShortcutGroup::Card_Counters)},
        {"Player/aCCYellow", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add yellow counter"),
                                         parseSequenceString(""),
                                         ShortcutGroup::Card_Counters)},
        {"Player/aRCYellow", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove yellow counter"),
                                         parseSequenceString(""),
                                         ShortcutGroup::Card_Counters)},
        {"Player/aSCYellow", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set yellow counters"),
                                         parseSequenceString(""),
                                         ShortcutGroup::Card_Counters)},
        {"Player/aCCRed", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add red counter"),
                                      parseSequenceString(""),
                                      ShortcutGroup::Card_Counters)},
        {"Player/aRCRed", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove red counter"),
                                      parseSequenceString(""),
                                      ShortcutGroup::Card_Counters)},
        {"Player/aSCRed", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set red counters"),
                                      parseSequenceString(""),
                                      ShortcutGroup::Card_Counters)},
        {"Player/aInc", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add life counter"),
                                    parseSequenceString("F12"),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aDec", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove life counter"),
                                    parseSequenceString("F11"),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aSet", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set life counters"),
                                    parseSequenceString("Ctrl+L"),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aIncCounter_w", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add white counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aDecCounter_w", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove white counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aSetCounter_w", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set white counters"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aIncCounter_u", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add blue counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aDecCounter_u", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove blue counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aSetCounter_u", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set blue counters"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aIncCounter_b", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add black counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aDecCounter_b", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove black counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aSetCounter_b", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set black counters"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aIncCounter_r", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add red counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aDecCounter_r", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove red counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aSetCounter_r", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set red counters"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aIncCounter_g", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add green counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aDecCounter_g", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove green counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aSetCounter_g", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set green counters"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aIncCounter_x", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add colorless counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aDecCounter_x", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove colorless counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aSetCounter_x", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set colorless counter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Player_Counters)},
        {"Player/aIncCounter_storm", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add storm counter"),
                                                 parseSequenceString(""),
                                                 ShortcutGroup::Player_Counters)},
        {"Player/aDecCounter_storm", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove storm counter"),
                                                 parseSequenceString(""),
                                                 ShortcutGroup::Player_Counters)},
        {"Player/aSetCounter_storm", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set storm counters"),
                                                 parseSequenceString(""),
                                                 ShortcutGroup::Player_Counters)},
        {"Player/aIncP", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add power (+1/+0)"),
                                     parseSequenceString("Ctrl++"),
                                     ShortcutGroup::Power_Toughness)},
        {"Player/aDecP", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove power (-1/-0)"),
                                     parseSequenceString("Ctrl+-"),
                                     ShortcutGroup::Power_Toughness)},
        {"Player/aFlowP", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Move toughness to power (+1/-1)"),
                                      parseSequenceString(""),
                                      ShortcutGroup::Power_Toughness)},
        {"Player/aIncT", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add toughness (+0/+1)"),
                                     parseSequenceString("Alt++"),
                                     ShortcutGroup::Power_Toughness)},
        {"Player/aDecT", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove toughness (-0/-1)"),
                                     parseSequenceString("Alt+-"),
                                     ShortcutGroup::Power_Toughness)},
        {"Player/aFlowT", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Move power to toughness (-1/+1)"),
                                      parseSequenceString(""),
                                      ShortcutGroup::Power_Toughness)},
        {"Player/aIncPT", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Add power and toughness (+1/+1)"),
                                      parseSequenceString("Ctrl+Alt++"),
                                      ShortcutGroup::Power_Toughness)},
        {"Player/aDecPT", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove power and toughness (-1/-1)"),
                                      parseSequenceString("Ctrl+Alt+-"),
                                      ShortcutGroup::Power_Toughness)},
        {"Player/aSetPT", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set power and toughness"),
                                      parseSequenceString("Ctrl+P"),
                                      ShortcutGroup::Power_Toughness)},
        {"Player/aResetPT", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Reset power and toughness"),
                                        parseSequenceString("Ctrl+Alt+0"),
                                        ShortcutGroup::Power_Toughness)},
        {"Player/phase0", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Untap"),
                                      parseSequenceString("F5"),
                                      ShortcutGroup::Game_Phases)},
        {"Player/phase1",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Upkeep"), parseSequenceString(""), ShortcutGroup::Game_Phases)},
        {"Player/phase2",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Draw"), parseSequenceString("F6"), ShortcutGroup::Game_Phases)},
        {"Player/phase3", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "First main phase"),
                                      parseSequenceString("F7"),
                                      ShortcutGroup::Game_Phases)},
        {"Player/phase4", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Start combat"),
                                      parseSequenceString("F8"),
                                      ShortcutGroup::Game_Phases)},
        {"Player/phase5",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Attack"), parseSequenceString(""), ShortcutGroup::Game_Phases)},
        {"Player/phase6",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Block"), parseSequenceString(""), ShortcutGroup::Game_Phases)},
        {"Player/phase7",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Damage"), parseSequenceString(""), ShortcutGroup::Game_Phases)},
        {"Player/phase8", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "End combat"),
                                      parseSequenceString(""),
                                      ShortcutGroup::Game_Phases)},
        {"Player/phase9", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Second main phase"),
                                      parseSequenceString("F9"),
                                      ShortcutGroup::Game_Phases)},
        {"Player/phase10",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "End"), parseSequenceString("F10"), ShortcutGroup::Game_Phases)},
        {"Player/aNextPhase", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Next phase"),
                                          parseSequenceString("Ctrl+Space;Tab"),
                                          ShortcutGroup::Game_Phases)},
        {"Player/aNextPhaseAction", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Next phase action"),
                                                parseSequenceString("Shift+Tab"),
                                                ShortcutGroup::Game_Phases)},
        {"Player/aNextTurn", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Next turn"),
                                         parseSequenceString("Ctrl+Return;Ctrl+Enter"),
                                         ShortcutGroup::Game_Phases)},
        {"Player/aTap", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Tap / Untap Card"),
                                    parseSequenceString(""),
                                    ShortcutGroup::Playing_Area)},
        {"Player/aUntapAll", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Untap all"),
                                         parseSequenceString("Ctrl+U"),
                                         ShortcutGroup::Playing_Area)},
        {"Player/aDoesntUntap", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Toggle untap"),
                                            parseSequenceString(""),
                                            ShortcutGroup::Playing_Area)},
        {"Player/aFlip", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Turn card over"),
                                     parseSequenceString(""),
                                     ShortcutGroup::Playing_Area)},
        {"Player/aPeek", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Peek card"),
                                     parseSequenceString(""),
                                     ShortcutGroup::Playing_Area)},
        {"Player/aPlay", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Play card"),
                                     parseSequenceString(""),
                                     ShortcutGroup::Playing_Area)},
        {"Player/aAttach", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Attach card"),
                                       parseSequenceString("Ctrl+Alt+A"),
                                       ShortcutGroup::Playing_Area)},
        {"Player/aUnattach", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Unattach card"),
                                         parseSequenceString(""),
                                         ShortcutGroup::Playing_Area)},
        {"Player/aClone", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Clone card"),
                                      parseSequenceString("Ctrl+J"),
                                      ShortcutGroup::Playing_Area)},
        {"Player/aCreateToken", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Create token"),
                                            parseSequenceString("Ctrl+T"),
                                            ShortcutGroup::Playing_Area)},
        {"Player/aCreateRelatedTokens", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Create all related tokens"),
                                                    parseSequenceString("Ctrl+Shift+T"),
                                                    ShortcutGroup::Playing_Area)},
        {"Player/aCreateAnotherToken", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Create another token"),
                                                   parseSequenceString("Ctrl+G"),
                                                   ShortcutGroup::Playing_Area)},
        {"Player/aSetAnnotation", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Set annotation"),
                                              parseSequenceString(""),
                                              ShortcutGroup::Playing_Area)},
        {"Player/aMoveToBottomLibrary", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Bottom library"),
                                                    parseSequenceString(""),
                                                    ShortcutGroup::Move_selected)},
        {"Player/aMoveToExile", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Exile"),
                                            parseSequenceString(""),
                                            ShortcutGroup::Move_selected)},
        {"Player/aMoveToGraveyard", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Graveyard"),
                                                parseSequenceString("Ctrl+Del"),
                                                ShortcutGroup::Move_selected)},
        {"Player/aMoveToHand",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Hand"), parseSequenceString(""), ShortcutGroup::Move_selected)},
        {"Player/aMoveToTopLibrary", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Top of library"),
                                                 parseSequenceString(""),
                                                 ShortcutGroup::Move_selected)},
        {"Player/aMoveTopToPlayFaceDown", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Play face down"),
                                                      parseSequenceString("Ctrl+Shift+E"),
                                                      ShortcutGroup::Move_selected)},
        {"Player/aViewGraveyard",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Graveyard"), parseSequenceString("F4"), ShortcutGroup::View)},
        {"Player/aViewLibrary",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Library"), parseSequenceString("F3"), ShortcutGroup::View)},
        {"Player/aViewRfg",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Exile"), parseSequenceString(""), ShortcutGroup::View)},
        {"Player/aViewSideboard", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Sideboard"),
                                              parseSequenceString("Ctrl+F3"),
                                              ShortcutGroup::View)},
        {"Player/aViewTopCards", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Top cards of library"),
                                             parseSequenceString("Ctrl+W"),
                                             ShortcutGroup::View)},
        {"Player/aCloseMostRecentZoneView", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Close recent view"),
                                                        parseSequenceString("Esc"),
                                                        ShortcutGroup::View)},
        {"Player/aMoveTopCardToGraveyard", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Move to graveyard once"),
                                                       parseSequenceString(""),
                                                       ShortcutGroup::Move_top)},
        {"Player/aMoveTopCardsToGraveyard", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Move multiple to graveyard"),
                                                        parseSequenceString("Ctrl+Shift+M"),
                                                        ShortcutGroup::Move_top)},
        {"Player/aMoveTopCardToExile", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Move to exile once"),
                                                   parseSequenceString(""),
                                                   ShortcutGroup::Move_top)},
        {"Player/aMoveTopCardsToExile", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Move multiple to exile"),
                                                    parseSequenceString(""),
                                                    ShortcutGroup::Move_top)},
        {"Player/aDrawArrow", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Draw arrow"),
                                          parseSequenceString(""),
                                          ShortcutGroup::Gameplay)},
        {"Player/aRemoveLocalArrows", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Remove local arrows"),
                                                  parseSequenceString("Ctrl+R"),
                                                  ShortcutGroup::Gameplay)},
        {"Player/aLeaveGame", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Leave game"),
                                          parseSequenceString("Ctrl+Q"),
                                          ShortcutGroup::Gameplay)},
        {"Player/aConcede",
         ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Concede"), parseSequenceString("F2"), ShortcutGroup::Gameplay)},
        {"Player/aRollDie", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Roll dice"),
                                        parseSequenceString("Ctrl+I"),
                                        ShortcutGroup::Gameplay)},
        {"Player/aShuffle", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Shuffle library"),
                                        parseSequenceString("Ctrl+S"),
                                        ShortcutGroup::Gameplay)},
        {"Player/aMulligan", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Mulligan"),
                                         parseSequenceString("Ctrl+M"),
                                         ShortcutGroup::Drawing)},
        {"Player/aDrawCard", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Draw a card"),
                                         parseSequenceString("Ctrl+D"),
                                         ShortcutGroup::Drawing)},
        {"Player/aDrawCards", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Draw multiple cards"),
                                          parseSequenceString("Ctrl+E"),
                                          ShortcutGroup::Drawing)},
        {"Player/aUndoDraw", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Undo draw"),
                                         parseSequenceString("Ctrl+Shift+D"),
                                         ShortcutGroup::Drawing)},
        {"Player/aAlwaysRevealTopCard", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Always reveal top card"),
                                                    parseSequenceString("Ctrl+N"),
                                                    ShortcutGroup::Drawing)},
        {"Player/aRotateViewCW", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Rotate view clockwise"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Drawing)},
        {"Player/aRotateViewCCW", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Rotate view counterclockwise"),
                                              parseSequenceString(""),
                                              ShortcutGroup::Drawing)},
        {"tab_room/aClearChat", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Clear chat"),
                                            parseSequenceString("F12"),
                                            ShortcutGroup::Chat_room)},
        {"Player/aResetLayout", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Reset layout"),
                                            parseSequenceString(""),
                                            ShortcutGroup::Game_window)},
        {"DlgLoadDeckFromClipboard/refreshButton", ShortcutKey(QT_TRANSLATE_NOOP("shortcutsTab", "Refresh"),
                                                               parseSequenceString("F5"),
                                                               ShortcutGroup::Load_deck)}};
};

#endif // SHORTCUTSSETTINGS_H
