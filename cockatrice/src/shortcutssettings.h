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
    const QString custom = "Custom";
    QString settingsFilePath;
    QHash<QString, ShortcutKey> shortCuts;

    QString stringifySequence(const QList<QKeySequence> &Sequence) const;
    QList<QKeySequence> parseSequenceString(const QString &stringSequence) const;

    typedef ShortcutKey SK;
    const QHash<QString, ShortcutKey> defaultShortCuts = {
        {"MainWindow/aCheckCardUpdates", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Check for card updates"),
                                            parseSequenceString(""),
                                            ShortcutGroup::Main_Window)},
        {"MainWindow/aConnect",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Connect"), parseSequenceString("Ctrl+L"), ShortcutGroup::Main_Window)},
        {"MainWindow/aDeckEditor",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Deck editor"), parseSequenceString(""), ShortcutGroup::Main_Window)},
        {"MainWindow/aDisconnect",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Disconnect"), parseSequenceString(""), ShortcutGroup::Main_Window)},
        {"MainWindow/aExit",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Exit"), parseSequenceString(""), ShortcutGroup::Main_Window)},
        {"MainWindow/aFullScreen", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Full screen"),
                                      parseSequenceString("Ctrl+F"),
                                      ShortcutGroup::Main_Window)},
        {"MainWindow/aRegister",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Register"), parseSequenceString(""), ShortcutGroup::Main_Window)},
        {"MainWindow/aSettings",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Settings"), parseSequenceString(""), ShortcutGroup::Main_Window)},
        {"MainWindow/aSinglePlayer",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Local gameplay"), parseSequenceString(""), ShortcutGroup::Main_Window)},
        {"MainWindow/aWatchReplay",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Watch replay"), parseSequenceString(""), ShortcutGroup::Main_Window)},
        {"TabDeckEditor/aAnalyzeDeck",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Analyze deck"), parseSequenceString(""), ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aClearFilterAll", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Clear all filters"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aClearFilterOne", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Clear selected filter"),
                                             parseSequenceString(""),
                                             ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aClose",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Close"), parseSequenceString(""), ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aDecrement",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Remove card"), parseSequenceString("-"), ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aManageSets",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Manage sets"), parseSequenceString(""), ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aEditTokens",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Edit tokens"), parseSequenceString(""), ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aExportDeckDecklist",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Export deck"), parseSequenceString(""), ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aIncrement",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Add card"), parseSequenceString("+"), ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aLoadDeck",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Load deck"), parseSequenceString("Ctrl+O"), ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aLoadDeckFromClipboard", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Load deck (clipboard)"),
                                                    parseSequenceString("Ctrl+Shift+V"),
                                                    ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aNewDeck",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "New deck"), parseSequenceString("Ctrl+N"), ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aOpenCustomFolder", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Open custom pic folder"),
                                               parseSequenceString(""),
                                               ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aPrintDeck", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Print deck"),
                                        parseSequenceString("Ctrl+P"),
                                        ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aRemoveCard",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Delete card"), parseSequenceString(""), ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aResetLayout",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Reset layout"), parseSequenceString(""), ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aSaveDeck",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Save deck"), parseSequenceString("Ctrl+S"), ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aSaveDeckAs",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Save deck as"), parseSequenceString(""), ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aSaveDeckToClipboard", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Save deck (clipboard; annotated)"),
                                                  parseSequenceString("Ctrl+Shift+C"),
                                                  ShortcutGroup::Deck_Editor)},
        {"TabDeckEditor/aSaveDeckToClipboardRaw", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Save deck (clipboard, raw)"),
                                                     parseSequenceString("Ctrl+Shift+R"),
                                                     ShortcutGroup::Deck_Editor)},
        {"DeckViewContainer/loadLocalButton", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Load local deck"),
                                                 parseSequenceString("Ctrl+O"),
                                                 ShortcutGroup::Game_Lobby)},
        {"DeckViewContainer/loadRemoteButton", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Load remote deck"),
                                                  parseSequenceString("Ctrl+Alt+O"),
                                                  ShortcutGroup::Game_Lobby)},
        {"Player/aCCGreen", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Add green counter"),
                               parseSequenceString(""),
                               ShortcutGroup::Card_Counters)},
        {"Player/aRCGreen", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Remove green counter"),
                               parseSequenceString(""),
                               ShortcutGroup::Card_Counters)},
        {"Player/aSCGreen", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Set green counters"),
                               parseSequenceString(""),
                               ShortcutGroup::Card_Counters)},
        {"Player/aCCYellow", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Add yellow counter"),
                                parseSequenceString(""),
                                ShortcutGroup::Card_Counters)},
        {"Player/aRCYellow", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Remove yellow counter"),
                                parseSequenceString(""),
                                ShortcutGroup::Card_Counters)},
        {"Player/aSCYellow", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Set yellow counters"),
                                parseSequenceString(""),
                                ShortcutGroup::Card_Counters)},
        {"Player/aCCRed", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Add red counter"),
                             parseSequenceString(""),
                             ShortcutGroup::Card_Counters)},
        {"Player/aRCRed", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Remove red counter"),
                             parseSequenceString(""),
                             ShortcutGroup::Card_Counters)},
        {"Player/aSCRed", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Set red counters"),
                             parseSequenceString(""),
                             ShortcutGroup::Card_Counters)},
        {"Player/aInc", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Add life counter"),
                           parseSequenceString("F12"),
                           ShortcutGroup::Player_Counters)},
        {"Player/aDec", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Remove life counter"),
                           parseSequenceString("F11"),
                           ShortcutGroup::Player_Counters)},
        {"Player/aSet", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Set life counters"),
                           parseSequenceString("Ctrl+L"),
                           ShortcutGroup::Player_Counters)},
        {"Player/aIncCounter_w", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Add white counter"),
                                    parseSequenceString(""),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aDecCounter_w", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Remove white counter"),
                                    parseSequenceString(""),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aSetCounter_w", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Set white counters"),
                                    parseSequenceString(""),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aIncCounter_u", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Add blue counter"),
                                    parseSequenceString(""),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aDecCounter_u", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Remove blue counter"),
                                    parseSequenceString(""),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aSetCounter_u", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Set blue counters"),
                                    parseSequenceString(""),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aIncCounter_b", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Add black counter"),
                                    parseSequenceString(""),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aDecCounter_b", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Remove black counter"),
                                    parseSequenceString(""),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aSetCounter_b", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Set black counters"),
                                    parseSequenceString(""),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aIncCounter_r", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Add red counter"),
                                    parseSequenceString(""),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aDecCounter_r", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Remove red counter"),
                                    parseSequenceString(""),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aSetCounter_r", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Set red counters"),
                                    parseSequenceString(""),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aIncCounter_g", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Add green counter"),
                                    parseSequenceString(""),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aDecCounter_g", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Remove green counter"),
                                    parseSequenceString(""),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aSetCounter_g", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Set green counters"),
                                    parseSequenceString(""),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aIncCounter_x", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Add X counter"),
                                    parseSequenceString(""),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aDecCounter_x", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Remove X counter"),
                                    parseSequenceString(""),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aSetCounter_x", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Set X counter"),
                                    parseSequenceString(""),
                                    ShortcutGroup::Player_Counters)},
        {"Player/aIncCounter_storm", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Add storm counter"),
                                        parseSequenceString(""),
                                        ShortcutGroup::Player_Counters)},
        {"Player/aDecCounter_storm", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Remove storm counter"),
                                        parseSequenceString(""),
                                        ShortcutGroup::Player_Counters)},
        {"Player/aSetCounter_storm", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Set storm counters"),
                                        parseSequenceString(""),
                                        ShortcutGroup::Player_Counters)},
        {"Player/aIncP", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Add power (+1/+0)"),
                            parseSequenceString("Ctrl++"),
                            ShortcutGroup::Power_Toughness)},
        {"Player/aDecP", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Remove power (-1/-0)"),
                            parseSequenceString("Ctrl+-"),
                            ShortcutGroup::Power_Toughness)},
        {"Player/aFlowP", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Move to power (+1/-1)"),
                             parseSequenceString(""),
                             ShortcutGroup::Power_Toughness)},
        {"Player/aIncT", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Add toughness (+0/+1)"),
                            parseSequenceString("Alt++"),
                            ShortcutGroup::Power_Toughness)},
        {"Player/aDecT", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Remove toughness (-0/-1)"),
                            parseSequenceString("Alt+-"),
                            ShortcutGroup::Power_Toughness)},
        {"Player/aFlowT", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Move to toughness (-1/+1)"),
                             parseSequenceString(""),
                             ShortcutGroup::Power_Toughness)},
        {"Player/aIncPT", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Add power and toughness (+1/+1)"),
                             parseSequenceString("Ctrl+Alt++"),
                             ShortcutGroup::Power_Toughness)},
        {"Player/aDecPT", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Remove power and toughness (-1/-1)"),
                             parseSequenceString("Ctrl+Alt+-"),
                             ShortcutGroup::Power_Toughness)},
        {"Player/aSetPT", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Set power and toughness"),
                             parseSequenceString("Ctrl+P"),
                             ShortcutGroup::Power_Toughness)},
        {"Player/aResetPT", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Reset power and toughness"),
                               parseSequenceString("Ctrl+Alt+0"),
                               ShortcutGroup::Power_Toughness)},
        {"Player/phase0",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Untap"), parseSequenceString("F5"), ShortcutGroup::Game_Phases)},
        {"Player/phase1",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Upkeep"), parseSequenceString(""), ShortcutGroup::Game_Phases)},
        {"Player/phase2",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Draw"), parseSequenceString("F6"), ShortcutGroup::Game_Phases)},
        {"Player/phase3",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Main 1"), parseSequenceString("F7"), ShortcutGroup::Game_Phases)},
        {"Player/phase4",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Start combat"), parseSequenceString("F8"), ShortcutGroup::Game_Phases)},
        {"Player/phase5",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Attack"), parseSequenceString(""), ShortcutGroup::Game_Phases)},
        {"Player/phase6",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Block"), parseSequenceString(""), ShortcutGroup::Game_Phases)},
        {"Player/phase7",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Damage"), parseSequenceString(""), ShortcutGroup::Game_Phases)},
        {"Player/phase8",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "End combat"), parseSequenceString(""), ShortcutGroup::Game_Phases)},
        {"Player/phase9",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Main 2"), parseSequenceString("F9"), ShortcutGroup::Game_Phases)},
        {"Player/phase10",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "End"), parseSequenceString("F10"), ShortcutGroup::Game_Phases)},
        {"Player/aNextPhase", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Next phase"),
                                 parseSequenceString("Ctrl+Space;Tab"),
                                 ShortcutGroup::Game_Phases)},
        {"Player/aNextPhaseAction", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Next phase action"),
                                       parseSequenceString("Shift+Tab"),
                                       ShortcutGroup::Game_Phases)},
        {"Player/aNextTurn", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Next turn"),
                                parseSequenceString("Ctrl+Return;Ctrl+Enter"),
                                ShortcutGroup::Game_Phases)},
        {"Player/aTap", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Tap / Untap Card"),
                           parseSequenceString(""),
                           ShortcutGroup::Playing_Area)},
        {"Player/aUntapAll", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Untap all"),
                                parseSequenceString("Ctrl+U"),
                                ShortcutGroup::Playing_Area)},
        {"Player/aDoesntUntap",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Toggle untap"), parseSequenceString(""), ShortcutGroup::Playing_Area)},
        {"Player/aFlip",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Flip card"), parseSequenceString(""), ShortcutGroup::Playing_Area)},
        {"Player/aPeek",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Peek card"), parseSequenceString(""), ShortcutGroup::Playing_Area)},
        {"Player/aPlay",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Play card"), parseSequenceString(""), ShortcutGroup::Playing_Area)},
        {"Player/aAttach", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Attach card"),
                              parseSequenceString("Ctrl+Alt+A"),
                              ShortcutGroup::Playing_Area)},
        {"Player/aUnattach",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Unattach card"), parseSequenceString(""), ShortcutGroup::Playing_Area)},
        {"Player/aClone", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Clone card"),
                             parseSequenceString("Ctrl+J"),
                             ShortcutGroup::Playing_Area)},
        {"Player/aCreateToken", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Create token"),
                                   parseSequenceString("Ctrl+T"),
                                   ShortcutGroup::Playing_Area)},
        {"Player/aCreateRelatedTokens", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Create all related tokens"),
                                           parseSequenceString("Ctrl+Shift+T"),
                                           ShortcutGroup::Playing_Area)},
        {"Player/aCreateAnotherToken", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Create another token"),
                                          parseSequenceString("Ctrl+G"),
                                          ShortcutGroup::Playing_Area)},
        {"Player/aSetAnnotation",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Set annotation"), parseSequenceString(""), ShortcutGroup::Playing_Area)},
        {"Player/aMoveToBottomLibrary", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Bottom library"),
                                           parseSequenceString(""),
                                           ShortcutGroup::Move_selected)},
        {"Player/aMoveToExile",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Exile"), parseSequenceString(""), ShortcutGroup::Move_selected)},
        {"Player/aMoveToGraveyard", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Graveyard"),
                                       parseSequenceString("Ctrl+Del"),
                                       ShortcutGroup::Move_selected)},
        {"Player/aMoveToHand",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Hand"), parseSequenceString(""), ShortcutGroup::Move_selected)},
        {"Player/aMoveToTopLibrary",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Top library"), parseSequenceString(""), ShortcutGroup::Move_selected)},
        {"Player/aMoveTopToPlayFaceDown", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Play face down"),
                                             parseSequenceString("Ctrl+Shift+E"),
                                             ShortcutGroup::Move_selected)},
        {"Player/aViewGraveyard",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Graveyard"), parseSequenceString("F4"), ShortcutGroup::View)},
        {"Player/aViewLibrary",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Library"), parseSequenceString("F3"), ShortcutGroup::View)},
        {"Player/aViewRfg",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Exile"), parseSequenceString(""), ShortcutGroup::View)},
        {"Player/aViewSideboard",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Sideboard"), parseSequenceString("Ctrl+F3"), ShortcutGroup::View)},
        {"Player/aViewTopCards", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Top cards of library"),
                                    parseSequenceString("Ctrl+W"),
                                    ShortcutGroup::View)},
        {"Player/aCloseMostRecentZoneView",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Close recent view"), parseSequenceString("Esc"), ShortcutGroup::View)},
        {"Player/aMoveTopCardToGraveyard",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Graveyard Once"), parseSequenceString(""), ShortcutGroup::Move_top)},
        {"Player/aMoveTopCardsToGraveyard", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Graveyard Multiple"),
                                               parseSequenceString("Ctrl+Shift+M"),
                                               ShortcutGroup::Move_top)},
        {"Player/aMoveTopCardToExile",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Exile Once"), parseSequenceString(""), ShortcutGroup::Move_top)},
        {"Player/aMoveTopCardsToExile",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Exile Multiple"), parseSequenceString(""), ShortcutGroup::Move_top)},
        {"Player/aDrawArrow",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Draw arrow"), parseSequenceString(""), ShortcutGroup::Gameplay)},
        {"Player/aRemoveLocalArrows", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Remove local arrows"),
                                         parseSequenceString("Ctrl+R"),
                                         ShortcutGroup::Gameplay)},
        {"Player/aLeaveGame",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Leave game"), parseSequenceString("Ctrl+Q"), ShortcutGroup::Gameplay)},
        {"Player/aConcede",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Concede"), parseSequenceString("F2"), ShortcutGroup::Gameplay)},
        {"Player/aRollDie",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Roll dice"), parseSequenceString("Ctrl+I"), ShortcutGroup::Gameplay)},
        {"Player/aShuffle", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Shuffle library"),
                               parseSequenceString("Ctrl+S"),
                               ShortcutGroup::Gameplay)},
        {"Player/aMulligan",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Mulligan"), parseSequenceString("Ctrl+M"), ShortcutGroup::Drawing)},
        {"Player/aDrawCard",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Draw card"), parseSequenceString("Ctrl+D"), ShortcutGroup::Drawing)},
        {"Player/aDrawCards",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Draw cards"), parseSequenceString("Ctrl+E"), ShortcutGroup::Drawing)},
        {"Player/aUndoDraw", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Undo draw"),
                                parseSequenceString("Ctrl+Shift+D"),
                                ShortcutGroup::Drawing)},
        {"Player/aAlwaysRevealTopCard", SK(QT_TRANSLATE_NOOP("shortcutsTab", "Always reveal top card"),
                                           parseSequenceString("Ctrl+N"),
                                           ShortcutGroup::Drawing)},
        {"Player/aRotateViewCW",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Rotate view CW"), parseSequenceString(""), ShortcutGroup::Drawing)},
        {"Player/aRotateViewCCW",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Rotate view CCW"), parseSequenceString(""), ShortcutGroup::Drawing)},
        {"tab_room/aClearChat",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Clear chat"), parseSequenceString("F12"), ShortcutGroup::Chat_room)},
        {"Player/aResetLayout",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Reset layout"), parseSequenceString(""), ShortcutGroup::Game_window)},
        {"DlgLoadDeckFromClipboard/refreshButton",
         SK(QT_TRANSLATE_NOOP("shortcutsTab", "Refresh"), parseSequenceString("F5"), ShortcutGroup::Load_deck)}};
};

#endif // SHORTCUTSSETTINGS_H
