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

QString ShortcutsSettings::getDefaultShortcutString(QString name)
{
    return stringifySecuence(defaultShortCuts.value(name));
}

QString ShortcutsSettings::getShortcutString(QString name)
{
    return stringifySecuence(shortCuts.value(name));
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

void ShortcutsSettings::setShortcuts(QString name, QString secuences)
{
    setShortcuts(name,parseSecuenceString(secuences));
}

bool ShortcutsSettings::isValid(QString name, QString secuences)
{
    QString checkKey = name.left(name.indexOf("/"));

    QStringList stringSecuences = secuences.split(";");

    QList<QString> allKeys = shortCuts.keys();
    for(int i=0; i < allKeys.size(); i++){
        QString key = allKeys.at(i);
        if(key.startsWith(checkKey) || key.startsWith("MainWindow") || checkKey.startsWith("MainWindow"))
        {
            QString storedSecuence = stringifySecuence(shortCuts.value(key));
            for(int j = 0; j < stringSecuences.size(); j++)
            {
                if(storedSecuence.contains(stringSecuences.at(j)))
                    return false;
            }
        }
    }
    return true;
}

void ShortcutsSettings::fillDefaultShorcuts()
{
    defaultShortCuts["MainWindow/aCheckCardUpdates"] = parseSecuenceString("");
    defaultShortCuts["MainWindow/aConnect"] = parseSecuenceString("");
    defaultShortCuts["MainWindow/aDeckEditor"] = parseSecuenceString("");
    defaultShortCuts["MainWindow/aDisconnect"] = parseSecuenceString("");
    defaultShortCuts["MainWindow/aExit"] = parseSecuenceString("");
    defaultShortCuts["MainWindow/aFullScreen"] = parseSecuenceString("Ctrl+F");
    defaultShortCuts["MainWindow/aRegister"] = parseSecuenceString("");
    defaultShortCuts["MainWindow/aSettings"] = parseSecuenceString("");
    defaultShortCuts["MainWindow/aSinglePlayer"] = parseSecuenceString("");
    defaultShortCuts["MainWindow/aWatchReplay"] = parseSecuenceString("");

    defaultShortCuts["TabDeckEditor/aAnalyzeDeck"] = parseSecuenceString("");
    defaultShortCuts["TabDeckEditor/aClearFilterAll"] = parseSecuenceString("");
    defaultShortCuts["TabDeckEditor/aClearFilterOne"] = parseSecuenceString("");
    defaultShortCuts["TabDeckEditor/aClose"] = parseSecuenceString("");
    defaultShortCuts["TabDeckEditor/aDecrement"] = parseSecuenceString("-");
    defaultShortCuts["TabDeckEditor/aEditSets"] = parseSecuenceString("");
    defaultShortCuts["TabDeckEditor/aEditTokens"] = parseSecuenceString("");
    defaultShortCuts["TabDeckEditor/aIncrement"] = parseSecuenceString("+");
    defaultShortCuts["TabDeckEditor/aLoadDeck"] = parseSecuenceString("Ctrl+O");
    defaultShortCuts["TabDeckEditor/aLoadDeckFromClipboard"] = parseSecuenceString("Ctrl+V");
    defaultShortCuts["TabDeckEditor/aNewDeck"] = parseSecuenceString("Ctrl+N");
    defaultShortCuts["TabDeckEditor/aOpenCustomFolder"] = parseSecuenceString("");
    defaultShortCuts["TabDeckEditor/aPrintDeck"] = parseSecuenceString("Ctrl+P");
    defaultShortCuts["TabDeckEditor/aRemoveCard"] = parseSecuenceString("Del");
    defaultShortCuts["TabDeckEditor/aResetLayout"] = parseSecuenceString("");
    defaultShortCuts["TabDeckEditor/aSaveDeck"] = parseSecuenceString("Ctrl+S");
    defaultShortCuts["TabDeckEditor/aSaveDeckAs"] = parseSecuenceString("");
    defaultShortCuts["TabDeckEditor/aSaveDeckToClipboard"] = parseSecuenceString("Ctrl+C");

    defaultShortCuts["DeckViewContainer/loadLocalButton"] = parseSecuenceString("Ctrl+O");
    defaultShortCuts["DeckViewContainer/loadRemoteButton"] = parseSecuenceString("Ctrl+Alt+O");

    defaultShortCuts["Player/aDec"] = parseSecuenceString("F11");
    defaultShortCuts["Player/aInc"] = parseSecuenceString("F12");
    defaultShortCuts["Player/aSet"] = parseSecuenceString("Ctrl+L");
    defaultShortCuts["Player/aCloseMostRecentZoneView"] = parseSecuenceString("Esc");
    defaultShortCuts["Player/IncP"] = parseSecuenceString("Ctrl++");
    defaultShortCuts["Player/aAlwaysRevealTopCard"] = parseSecuenceString("Ctrl+N");
    defaultShortCuts["Player/aAttach"] = parseSecuenceString("Ctrl+A");
    defaultShortCuts["Player/aCCGreen"] = parseSecuenceString("");
    defaultShortCuts["Player/aCCRed"] = parseSecuenceString("");
    defaultShortCuts["Player/aCCYellow"] = parseSecuenceString("");
    defaultShortCuts["Player/aClone"] = parseSecuenceString("Ctrl+J");
    defaultShortCuts["Player/aCreateAnotherToken"] = parseSecuenceString("Ctrl+G");
    defaultShortCuts["Player/aCreateToken"] = parseSecuenceString("Ctrl+T");
    defaultShortCuts["Player/aDecP"] = parseSecuenceString("Ctrl+-");
    defaultShortCuts["Player/aDecPT"] = parseSecuenceString("Ctrl+Alt+-");
    defaultShortCuts["Player/aDecT"] = parseSecuenceString("Alt+-");
    defaultShortCuts["Player/aDoesntUntap"] = parseSecuenceString("");
    defaultShortCuts["Player/aDrawArrow"] = parseSecuenceString("");
    defaultShortCuts["Player/aDrawCard"] = parseSecuenceString("Ctrl+D");
    defaultShortCuts["Player/aDrawCards"] = parseSecuenceString("Ctrl+E");
    defaultShortCuts["Player/aFlip"] = parseSecuenceString("");
    defaultShortCuts["Player/aIncPT"] = parseSecuenceString("Ctrl+Alt++");
    defaultShortCuts["Player/aIncT"] = parseSecuenceString("Alt++");
    defaultShortCuts["Player/aMoveToBottomLibrary"] = parseSecuenceString("");
    defaultShortCuts["Player/aMoveToExile"] = parseSecuenceString("");
    defaultShortCuts["Player/aMoveToGraveyard"] = parseSecuenceString("Ctrl+Del");
    defaultShortCuts["Player/aMoveToHand"] = parseSecuenceString("");
    defaultShortCuts["Player/aMoveToTopLibrary"] = parseSecuenceString("");
    defaultShortCuts["Player/aMulligan"] = parseSecuenceString("Ctrl+M");
    defaultShortCuts["Player/aPeek"] = parseSecuenceString("");
    defaultShortCuts["Player/aPlay"] = parseSecuenceString("");
    defaultShortCuts["Player/aRCGreen"] = parseSecuenceString("");
    defaultShortCuts["Player/aRCRed"] = parseSecuenceString("");
    defaultShortCuts["Player/aRCYellow"] = parseSecuenceString("");
    defaultShortCuts["Player/aRollDie"] = parseSecuenceString("Ctrl+I");
    defaultShortCuts["Player/aSCGreen"] = parseSecuenceString("");
    defaultShortCuts["Player/aSCRed"] = parseSecuenceString("");
    defaultShortCuts["Player/aSCYellow"] = parseSecuenceString("");
    defaultShortCuts["Player/aSetAnnotation"] = parseSecuenceString("");
    defaultShortCuts["Player/aSetPT"] = parseSecuenceString("Ctrl+P");
    defaultShortCuts["Player/aShuffle"] = parseSecuenceString("Ctrl+S");
    defaultShortCuts["Player/aTap"] = parseSecuenceString("");
    defaultShortCuts["Player/aUnattach"] = parseSecuenceString("");
    defaultShortCuts["Player/aUndoDraw"] = parseSecuenceString("Ctrl+Shift+D");
    defaultShortCuts["Player/aUntap"] = parseSecuenceString("");
    defaultShortCuts["Player/aUntapAll"] = parseSecuenceString("Ctrl+U");
    defaultShortCuts["Player/aViewGraveyard"] = parseSecuenceString("F4");
    defaultShortCuts["Player/aViewLibrary"] = parseSecuenceString("F3");
    defaultShortCuts["Player/aViewRfg"] = parseSecuenceString("");
    defaultShortCuts["Player/aViewSideboard"] = parseSecuenceString("Ctrl+F3");
    defaultShortCuts["Player/aViewTopCards"] = parseSecuenceString("Ctrl+W");
    defaultShortCuts["Player/aConcede"] = parseSecuenceString("F2");
    defaultShortCuts["Player/aLeaveGame"] = parseSecuenceString("Ctrl+Q");
    defaultShortCuts["Player/aNextPhase"] = parseSecuenceString("Ctrl+Space;Tab");
    defaultShortCuts["Player/aNextTurn"] = parseSecuenceString("Ctrl+Return;Ctrl+Enter");
    defaultShortCuts["Player/aRemoveLocalArrows"] = parseSecuenceString("Ctrl+R");
    defaultShortCuts["Player/aRotateViewCCW"] = parseSecuenceString("Ctrl+[");
    defaultShortCuts["Player/aRotateViewCW"] = parseSecuenceString("Ctrl+]");
    defaultShortCuts["Player/phase0"] = parseSecuenceString("F5");
    defaultShortCuts["Player/phase1"] = parseSecuenceString("");
    defaultShortCuts["Player/phase10"] = parseSecuenceString("F10");
    defaultShortCuts["Player/phase2"] = parseSecuenceString("F6");
    defaultShortCuts["Player/phase3"] = parseSecuenceString("F7");
    defaultShortCuts["Player/phase4"] = parseSecuenceString("F8");
    defaultShortCuts["Player/phase5"] = parseSecuenceString("");
    defaultShortCuts["Player/phase6"] = parseSecuenceString("");
    defaultShortCuts["Player/phase7"] = parseSecuenceString("");
    defaultShortCuts["Player/phase8"] = parseSecuenceString("");
    defaultShortCuts["Player/phase9"] = parseSecuenceString("F9");
}

