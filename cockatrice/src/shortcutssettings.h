#ifndef SHORTCUTSSETTINGS_H
#define SHORTCUTSSETTINGS_H

#include <QHash>
#include <QKeySequence>
#include <QObject>
#include <QSettings>

class ShortcutsSettings : public QObject
{
    Q_OBJECT
public:
    ShortcutsSettings(const QString &settingsFilePath, QObject *parent = nullptr);

    QList<QKeySequence> getDefaultShortcut(const QString &name) const;
    QList<QKeySequence> getShortcut(const QString &name) const;
    QKeySequence getSingleShortcut(const QString &name) const;
    QString getDefaultShortcutString(const QString &name) const;
    QString getShortcutString(const QString &name) const;

    void setShortcuts(const QString &name, const QList<QKeySequence> &Sequence);
    void setShortcuts(const QString &name, const QKeySequence &Sequence);
    void setShortcuts(const QString &name, const QString &Sequences);

    bool isKeyAllowed(const QString &name, const QString &Sequences) const;
    bool isValid(const QString &name, const QString &Sequences) const;

    void resetAllShortcuts();
    void clearAllShortcuts();

signals:
    void shortCutChanged();
    void allShortCutsReset();
    void allShortCutsClear();

private:
    const QChar sep = ';';
    const QString custom = "Custom";
    QString settingsFilePath;
    QHash<QString, QList<QKeySequence>> shortCuts;

    QString stringifySequence(const QList<QKeySequence> &Sequence) const;
    QList<QKeySequence> parseSequenceString(const QString &stringSequence) const;

    const QHash<QString, QList<QKeySequence>> defaultShortCuts{
        {"MainWindow/aCheckCardUpdates", parseSequenceString("")},
        {"MainWindow/aConnect", parseSequenceString("Ctrl+L")},
        {"MainWindow/aDeckEditor", parseSequenceString("")},
        {"MainWindow/aDisconnect", parseSequenceString("")},
        {"MainWindow/aExit", parseSequenceString("")},
        {"MainWindow/aFullScreen", parseSequenceString("Ctrl+F")},
        {"MainWindow/aRegister", parseSequenceString("")},
        {"MainWindow/aSettings", parseSequenceString("")},
        {"MainWindow/aSinglePlayer", parseSequenceString("")},
        {"MainWindow/aWatchReplay", parseSequenceString("")},

        {"TabDeckEditor/aAnalyzeDeck", parseSequenceString("")},
        {"TabDeckEditor/aClearFilterAll", parseSequenceString("")},
        {"TabDeckEditor/aClearFilterOne", parseSequenceString("")},
        {"TabDeckEditor/aClose", parseSequenceString("")},
        {"TabDeckEditor/aDecrement", parseSequenceString("-")},
        {"TabDeckEditor/aManageSets", parseSequenceString("")},
        {"TabDeckEditor/aEditTokens", parseSequenceString("")},
        {"TabDeckEditor/aExportDeckDecklist", parseSequenceString("")},
        {"TabDeckEditor/aIncrement", parseSequenceString("+")},
        {"TabDeckEditor/aLoadDeck", parseSequenceString("Ctrl+O")},
        {"TabDeckEditor/aLoadDeckFromClipboard", parseSequenceString("Ctrl+Shift+V")},
        {"TabDeckEditor/aNewDeck", parseSequenceString("Ctrl+N")},
        {"TabDeckEditor/aOpenCustomFolder", parseSequenceString("")},
        {"TabDeckEditor/aPrintDeck", parseSequenceString("Ctrl+P")},
        {"TabDeckEditor/aRemoveCard", parseSequenceString("")},
        {"TabDeckEditor/aResetLayout", parseSequenceString("")},
        {"TabDeckEditor/aSaveDeck", parseSequenceString("Ctrl+S")},
        {"TabDeckEditor/aSaveDeckAs", parseSequenceString("")},
        {"TabDeckEditor/aSaveDeckToClipboard", parseSequenceString("Ctrl+Shift+C")},
        {"TabDeckEditor/aSaveDeckToClipboardRaw", parseSequenceString("Ctrl+Shift+R")},

        {"DeckViewContainer/loadLocalButton", parseSequenceString("Ctrl+O")},
        {"DeckViewContainer/loadRemoteButton", parseSequenceString("Ctrl+Alt+O")},

        {"Player/aSet", parseSequenceString("Ctrl+L")},
        {"Player/aAlwaysRevealTopCard", parseSequenceString("Ctrl+N")},
        {"Player/aCloseMostRecentZoneView", parseSequenceString("Esc")},
        {"Player/aDrawCard", parseSequenceString("Ctrl+D")},
        {"Player/aDrawCards", parseSequenceString("Ctrl+E")},
        {"Player/aDec", parseSequenceString("F11")},
        {"Player/aInc", parseSequenceString("F12")},
        {"Player/aMoveTopCardToGraveyard", parseSequenceString("")},
        {"Player/aMoveTopCardsToGraveyard", parseSequenceString("Ctrl+Shift+M")},
        {"Player/aMoveTopCardToExile", parseSequenceString("")},
        {"Player/aMoveTopCardsToExile", parseSequenceString("")},
        {"Player/aMoveTopToPlayFaceDown", parseSequenceString("Ctrl+Shift+E")},
        {"Player/aMulligan", parseSequenceString("Ctrl+M")},
        {"Player/aPeek", parseSequenceString("")},
        {"Player/aPlay", parseSequenceString("")},
        {"Player/aResetLayout", parseSequenceString("")},
        {"Player/aRollDie", parseSequenceString("Ctrl+I")},
        {"Player/aShuffle", parseSequenceString("Ctrl+S")},
        {"Player/aUndoDraw", parseSequenceString("Ctrl+Shift+D")},
        {"Player/aUntapAll", parseSequenceString("Ctrl+U")},
        {"Player/aViewGraveyard", parseSequenceString("F4")},
        {"Player/aViewLibrary", parseSequenceString("F3")},
        {"Player/aViewRfg", parseSequenceString("")},
        {"Player/aViewSideboard", parseSequenceString("Ctrl+F3")},
        {"Player/aViewTopCards", parseSequenceString("Ctrl+W")},

        {"Player/aAttach", parseSequenceString("Ctrl+Alt+A")},
        {"Player/aClone", parseSequenceString("Ctrl+J")},
        {"Player/aCreateAnotherToken", parseSequenceString("Ctrl+G")},
        {"Player/aCreateToken", parseSequenceString("Ctrl+T")},
        {"Player/aCreateRelatedTokens", parseSequenceString("Ctrl+Shift+T")},
        {"Player/aDoesntUntap", parseSequenceString("")},
        {"Player/aDrawArrow", parseSequenceString("")},
        {"Player/aFlip", parseSequenceString("")},
        {"Player/aMoveToBottomLibrary", parseSequenceString("")},
        {"Player/aMoveToExile", parseSequenceString("")},
        {"Player/aMoveToGraveyard", parseSequenceString("Ctrl+Del")},
        {"Player/aMoveToHand", parseSequenceString("")},
        {"Player/aMoveToTopLibrary", parseSequenceString("")},
        {"Player/aSetAnnotation", parseSequenceString("")},
        {"Player/aTap", parseSequenceString("")},
        {"Player/aUnattach", parseSequenceString("")},

        {"Player/aCCGreen", parseSequenceString("")},
        {"Player/aCCRed", parseSequenceString("")},
        {"Player/aCCYellow", parseSequenceString("")},
        {"Player/aRCGreen", parseSequenceString("")},
        {"Player/aRCRed", parseSequenceString("")},
        {"Player/aRCYellow", parseSequenceString("")},
        {"Player/aSCGreen", parseSequenceString("")},
        {"Player/aSCRed", parseSequenceString("")},
        {"Player/aSCYellow", parseSequenceString("")},

        {"Player/aDecP", parseSequenceString("Ctrl+-")},
        {"Player/aDecPT", parseSequenceString("Ctrl+Alt+-")},
        {"Player/aDecT", parseSequenceString("Alt+-")},
        {"Player/aIncP", parseSequenceString("Ctrl++")},
        {"Player/aIncPT", parseSequenceString("Ctrl+Alt++")},
        {"Player/aIncT", parseSequenceString("Alt++")},
        {"Player/aSetPT", parseSequenceString("Ctrl+P")},
        {"Player/aResetPT", parseSequenceString("Ctrl+Alt+0")},

        {"Player/aConcede", parseSequenceString("F2")},
        {"Player/aLeaveGame", parseSequenceString("Ctrl+Q")},
        {"Player/aNextPhase", parseSequenceString("Ctrl+Space;Tab")},
        {"Player/aNextPhaseAction", parseSequenceString("Shift+Tab")},
        {"Player/aNextTurn", parseSequenceString("Ctrl+Return;Ctrl+Enter")},
        {"Player/aRemoveLocalArrows", parseSequenceString("Ctrl+R")},
        {"Player/aRotateViewCCW", parseSequenceString("")},
        {"Player/aRotateViewCW", parseSequenceString("")},
        {"Player/phase0", parseSequenceString("F5")},
        {"Player/phase1", parseSequenceString("")},
        {"Player/phase10", parseSequenceString("F10")},
        {"Player/phase2", parseSequenceString("F6")},
        {"Player/phase3", parseSequenceString("F7")},
        {"Player/phase4", parseSequenceString("F8")},
        {"Player/phase5", parseSequenceString("")},
        {"Player/phase6", parseSequenceString("")},
        {"Player/phase7", parseSequenceString("")},
        {"Player/phase8", parseSequenceString("")},
        {"Player/phase9", parseSequenceString("F9")},

        {"Player/aDecCounter_w", parseSequenceString("")},
        {"Player/aIncCounter_w", parseSequenceString("")},
        {"Player/aSetCounter_w", parseSequenceString("")},

        {"Player/aDecCounter_u", parseSequenceString("")},
        {"Player/aIncCounter_u", parseSequenceString("")},
        {"Player/aSetCounter_u", parseSequenceString("")},

        {"Player/aDecCounter_b", parseSequenceString("")},
        {"Player/aIncCounter_b", parseSequenceString("")},
        {"Player/aSetCounter_b", parseSequenceString("")},

        {"Player/aDecCounter_r", parseSequenceString("")},
        {"Player/aIncCounter_r", parseSequenceString("")},
        {"Player/aSetCounter_r", parseSequenceString("")},

        {"Player/aDecCounter_g", parseSequenceString("")},
        {"Player/aIncCounter_g", parseSequenceString("")},
        {"Player/aSetCounter_g", parseSequenceString("")},

        {"Player/aDecCounter_x", parseSequenceString("")},
        {"Player/aIncCounter_x", parseSequenceString("")},
        {"Player/aSetCounter_x", parseSequenceString("")},

        {"Player/aDecCounter_storm", parseSequenceString("")},
        {"Player/aIncCounter_storm", parseSequenceString("")},
        {"Player/aSetCounter_storm", parseSequenceString("")},

        {"tab_room/aClearChat", parseSequenceString("F12")},
        {"DlgLoadDeckFromClipboard/refreshButton", parseSequenceString("F5")}};
};

#endif // SHORTCUTSSETTINGS_H
