#include "shortcutssettings.h"
#include <QFile>
#include <QStringList>

ShortcutsSettings::ShortcutsSettings(QString settingsPath, QObject *parent) : QObject(parent)
{
    this->settingsFilePath = settingsPath;
    this->settingsFilePath.append("shortcuts.ini");
    fillDefaultShorcuts();
    shortCuts = QMap<QString,QList<QKeySequence> >(defaultShortCuts);

    bool exists = QFile(settingsFilePath).exists();

    QSettings shortCutsFile(settingsFilePath, QSettings::IniFormat);

    if(exists){
        shortCutsFile.beginGroup("Custom");
        const QStringList customKeys = shortCutsFile.allKeys();
        for(QStringList::const_iterator it = customKeys.constBegin(); it != customKeys.constEnd(); ++it)
        {
            QString stringSequence = shortCutsFile.value(*it).toString();
            QList<QKeySequence> SequenceList = parseSequenceString(stringSequence);
            shortCuts.insert(*it, SequenceList);
        }
        shortCutsFile.endGroup();
    }
}

QList<QKeySequence> ShortcutsSettings::getShortcut(QString name)
{
    if(shortCuts.contains(name))
        return shortCuts.value(name);

    return defaultShortCuts.value(name, QList<QKeySequence>());
}

QKeySequence ShortcutsSettings::getSingleShortcut(QString name)
{
    return getShortcut(name).at(0);
}

QString ShortcutsSettings::getDefaultShortcutString(QString name)
{
    return stringifySequence(defaultShortCuts.value(name));
}

QString ShortcutsSettings::getShortcutString(QString name)
{
    return stringifySequence(shortCuts.value(name));
}

QString ShortcutsSettings::stringifySequence(QList<QKeySequence> Sequence) const
{
    QString stringSequence;
    for(int i=0; i < Sequence.size(); ++i)
    {
        stringSequence.append(Sequence.at(i).toString(QKeySequence::PortableText));
        if(i < Sequence.size() - 1)
            stringSequence.append(";");
    }

    return stringSequence;
}

QList<QKeySequence> ShortcutsSettings::parseSequenceString(QString stringSequence)
{
    QStringList Sequences = stringSequence.split(";");
    QList<QKeySequence> SequenceList;
    for(QStringList::const_iterator ss = Sequences.constBegin(); ss != Sequences.constEnd(); ++ss)
    {
        SequenceList.append(QKeySequence(*ss, QKeySequence::PortableText));
    }

    return SequenceList;
}

void ShortcutsSettings::setShortcuts(QString name, QList<QKeySequence> Sequence)
{
    shortCuts[name] = Sequence;

    QSettings shortCutsFile(settingsFilePath, QSettings::IniFormat);
    shortCutsFile.beginGroup("Custom");
    QString stringSequence = stringifySequence(Sequence);
    shortCutsFile.setValue(name, stringSequence);
    shortCutsFile.endGroup();
    emit shortCutchanged();
}

void ShortcutsSettings::setShortcuts(QString name, QKeySequence Sequence)
{
    setShortcuts(name, QList<QKeySequence>() << Sequence);
}

void ShortcutsSettings::setShortcuts(QString name, QString Sequences)
{
    setShortcuts(name,parseSequenceString(Sequences));
}

bool ShortcutsSettings::isValid(QString name, QString Sequences)
{
    QString checkKey = name.left(name.indexOf("/"));

    QString checkSequence = Sequences.split(";").last();

    QList<QString> allKeys = shortCuts.keys();
    for(int i=0; i < allKeys.size(); i++){
        QString key = allKeys.at(i);
        if(key.startsWith(checkKey) || key.startsWith("MainWindow") || checkKey.startsWith("MainWindow"))
        {
            QString storedSequence = stringifySequence(shortCuts.value(key));
            QStringList stringSequences = storedSequence.split(";");
            for(int j = 0; j < stringSequences.size(); j++)
            {
                if(checkSequence == stringSequences.at(j))
                    return false;
            }
        }
    }
    return true;
}

void ShortcutsSettings::resetAllShortcuts()
{
   for(QMap<QString,QList<QKeySequence> >::const_iterator it = defaultShortCuts.begin();
                                                        it != defaultShortCuts.end(); ++it){
       setShortcuts(it.key(), it.value());
   }
   emit allShortCutsReset();
}

void ShortcutsSettings::clearAllShortcuts()
{
    for(QMap<QString,QList<QKeySequence> >::const_iterator it = shortCuts.begin();
                                                         it != shortCuts.end(); ++it){
        setShortcuts(it.key(), "");
    }
    emit allShortCutsClear();
}

void ShortcutsSettings::fillDefaultShorcuts()
{
    defaultShortCuts["MainWindow/aCheckCardUpdates"] = parseSequenceString("");
    defaultShortCuts["MainWindow/aConnect"] = parseSequenceString("Ctrl+L");
    defaultShortCuts["MainWindow/aDeckEditor"] = parseSequenceString("");
    defaultShortCuts["MainWindow/aDisconnect"] = parseSequenceString("");
    defaultShortCuts["MainWindow/aExit"] = parseSequenceString("");
    defaultShortCuts["MainWindow/aFullScreen"] = parseSequenceString("Ctrl+F");
    defaultShortCuts["MainWindow/aRegister"] = parseSequenceString("");
    defaultShortCuts["MainWindow/aSettings"] = parseSequenceString("");
    defaultShortCuts["MainWindow/aSinglePlayer"] = parseSequenceString("");
    defaultShortCuts["MainWindow/aWatchReplay"] = parseSequenceString("");

    defaultShortCuts["TabDeckEditor/aAnalyzeDeck"] = parseSequenceString("");
    defaultShortCuts["TabDeckEditor/aClearFilterAll"] = parseSequenceString("");
    defaultShortCuts["TabDeckEditor/aClearFilterOne"] = parseSequenceString("");
    defaultShortCuts["TabDeckEditor/aClose"] = parseSequenceString("");
    defaultShortCuts["TabDeckEditor/aDecrement"] = parseSequenceString("-");
    defaultShortCuts["TabDeckEditor/aEditSets"] = parseSequenceString("");
    defaultShortCuts["TabDeckEditor/aEditTokens"] = parseSequenceString("");
    defaultShortCuts["TabDeckEditor/aIncrement"] = parseSequenceString("+");
    defaultShortCuts["TabDeckEditor/aLoadDeck"] = parseSequenceString("Ctrl+O");
    defaultShortCuts["TabDeckEditor/aLoadDeckFromClipboard"] = parseSequenceString("Ctrl+Shift+V");
    defaultShortCuts["TabDeckEditor/aNewDeck"] = parseSequenceString("Ctrl+N");
    defaultShortCuts["TabDeckEditor/aOpenCustomFolder"] = parseSequenceString("");
    defaultShortCuts["TabDeckEditor/aPrintDeck"] = parseSequenceString("Ctrl+P");
    defaultShortCuts["TabDeckEditor/aRemoveCard"] = parseSequenceString("Del");
    defaultShortCuts["TabDeckEditor/aResetLayout"] = parseSequenceString("");
    defaultShortCuts["TabDeckEditor/aSaveDeck"] = parseSequenceString("Ctrl+S");
    defaultShortCuts["TabDeckEditor/aSaveDeckAs"] = parseSequenceString("");
    defaultShortCuts["TabDeckEditor/aSaveDeckToClipboard"] = parseSequenceString("Ctrl+Shift+C");

    defaultShortCuts["DeckViewContainer/loadLocalButton"] = parseSequenceString("Ctrl+O");
    defaultShortCuts["DeckViewContainer/loadRemoteButton"] = parseSequenceString("Ctrl+Alt+O");

    defaultShortCuts["Player/aDec"] = parseSequenceString("F11");
    defaultShortCuts["Player/aInc"] = parseSequenceString("F12");
    defaultShortCuts["Player/aSet"] = parseSequenceString("Ctrl+L");
    defaultShortCuts["Player/aCloseMostRecentZoneView"] = parseSequenceString("Esc");
    defaultShortCuts["Player/IncP"] = parseSequenceString("Ctrl++");
    defaultShortCuts["Player/aAlwaysRevealTopCard"] = parseSequenceString("Ctrl+N");
    defaultShortCuts["Player/aAttach"] = parseSequenceString("Ctrl+Alt+A");
    defaultShortCuts["Player/aCCGreen"] = parseSequenceString("");
    defaultShortCuts["Player/aCCRed"] = parseSequenceString("");
    defaultShortCuts["Player/aCCYellow"] = parseSequenceString("");
    defaultShortCuts["Player/aClone"] = parseSequenceString("Ctrl+J");
    defaultShortCuts["Player/aCreateAnotherToken"] = parseSequenceString("Ctrl+G");
    defaultShortCuts["Player/aCreateToken"] = parseSequenceString("Ctrl+T");
    defaultShortCuts["Player/aDecP"] = parseSequenceString("Ctrl+-");
    defaultShortCuts["Player/aDecPT"] = parseSequenceString("Ctrl+Alt+-");
    defaultShortCuts["Player/aDecT"] = parseSequenceString("Alt+-");
    defaultShortCuts["Player/aDoesntUntap"] = parseSequenceString("");
    defaultShortCuts["Player/aDrawArrow"] = parseSequenceString("");
    defaultShortCuts["Player/aDrawCard"] = parseSequenceString("Ctrl+D");
    defaultShortCuts["Player/aDrawCards"] = parseSequenceString("Ctrl+E");
    defaultShortCuts["Player/aFlip"] = parseSequenceString("");
    defaultShortCuts["Player/aIncPT"] = parseSequenceString("Ctrl+Alt++");
    defaultShortCuts["Player/aIncT"] = parseSequenceString("Alt++");
    defaultShortCuts["Player/aMoveToBottomLibrary"] = parseSequenceString("");
    defaultShortCuts["Player/aMoveToExile"] = parseSequenceString("");
    defaultShortCuts["Player/aMoveToGraveyard"] = parseSequenceString("Ctrl+Del");
    defaultShortCuts["Player/aMoveToHand"] = parseSequenceString("");
    defaultShortCuts["Player/aMoveToTopLibrary"] = parseSequenceString("");
    defaultShortCuts["Player/aMulligan"] = parseSequenceString("Ctrl+M");
    defaultShortCuts["Player/aPeek"] = parseSequenceString("");
    defaultShortCuts["Player/aPlay"] = parseSequenceString("");
    defaultShortCuts["Player/aRCGreen"] = parseSequenceString("");
    defaultShortCuts["Player/aRCRed"] = parseSequenceString("");
    defaultShortCuts["Player/aRCYellow"] = parseSequenceString("");
    defaultShortCuts["Player/aRollDie"] = parseSequenceString("Ctrl+I");
    defaultShortCuts["Player/aSCGreen"] = parseSequenceString("");
    defaultShortCuts["Player/aSCRed"] = parseSequenceString("");
    defaultShortCuts["Player/aSCYellow"] = parseSequenceString("");
    defaultShortCuts["Player/aSetAnnotation"] = parseSequenceString("");
    defaultShortCuts["Player/aSetPT"] = parseSequenceString("Ctrl+P");
    defaultShortCuts["Player/aShuffle"] = parseSequenceString("Ctrl+S");
    defaultShortCuts["Player/aTap"] = parseSequenceString("");
    defaultShortCuts["Player/aUnattach"] = parseSequenceString("");
    defaultShortCuts["Player/aUndoDraw"] = parseSequenceString("Ctrl+Shift+D");
    defaultShortCuts["Player/aUntap"] = parseSequenceString("");
    defaultShortCuts["Player/aUntapAll"] = parseSequenceString("Ctrl+U");
    defaultShortCuts["Player/aViewGraveyard"] = parseSequenceString("F4");
    defaultShortCuts["Player/aViewLibrary"] = parseSequenceString("F3");
    defaultShortCuts["Player/aViewRfg"] = parseSequenceString("");
    defaultShortCuts["Player/aViewSideboard"] = parseSequenceString("Ctrl+F3");
    defaultShortCuts["Player/aViewTopCards"] = parseSequenceString("Ctrl+W");
    defaultShortCuts["Player/aConcede"] = parseSequenceString("F2");
    defaultShortCuts["Player/aLeaveGame"] = parseSequenceString("Ctrl+Q");
    defaultShortCuts["Player/aNextPhase"] = parseSequenceString("Ctrl+Space;Tab");
    defaultShortCuts["Player/aNextTurn"] = parseSequenceString("Ctrl+Return;Ctrl+Enter");
    defaultShortCuts["Player/aRemoveLocalArrows"] = parseSequenceString("Ctrl+R");
    defaultShortCuts["Player/aRotateViewCCW"] = parseSequenceString("");
    defaultShortCuts["Player/aRotateViewCW"] = parseSequenceString("");
    defaultShortCuts["Player/phase0"] = parseSequenceString("F5");
    defaultShortCuts["Player/phase1"] = parseSequenceString("");
    defaultShortCuts["Player/phase10"] = parseSequenceString("F10");
    defaultShortCuts["Player/phase2"] = parseSequenceString("F6");
    defaultShortCuts["Player/phase3"] = parseSequenceString("F7");
    defaultShortCuts["Player/phase4"] = parseSequenceString("F8");
    defaultShortCuts["Player/phase5"] = parseSequenceString("");
    defaultShortCuts["Player/phase6"] = parseSequenceString("");
    defaultShortCuts["Player/phase7"] = parseSequenceString("");
    defaultShortCuts["Player/phase8"] = parseSequenceString("");
    defaultShortCuts["Player/phase9"] = parseSequenceString("F9");
    defaultShortCuts["tab_room/aClearChat"] = parseSequenceString("F12");
    defaultShortCuts["DlgLoadDeckFromClipboard/refreshButton"] = parseSequenceString("F5");
    defaultShortCuts["Player/aResetLayout"] = parseSequenceString("");
}
