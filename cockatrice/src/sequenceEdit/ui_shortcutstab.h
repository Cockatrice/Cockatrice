#ifndef UI_SHORTCUTSTAB_H
#define UI_SHORTCUTSTAB_H

#include "sequenceedit.h"
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <QtCore/QVariant>

QT_BEGIN_NAMESPACE

#define WIKI "https://github.com/Cockatrice/Cockatrice/wiki/Custom-Keyboard-Shortcuts"

class Ui_shortcutsTab
{
public:
    QGridLayout *gridLayout_9;
    QTabWidget *tabWidget;
    QWidget *tab;
    QGridLayout *gridLayout_3;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_2;
    QLabel *lbl_MainWindow_aDeckEditor;
    SequenceEdit *MainWindow_aDeckEditor;
    QLabel *lbl_MainWindow_aSinglePlayer;
    SequenceEdit *MainWindow_aSinglePlayer;
    QLabel *lbl_MainWindow_aWatchReplay;
    QLabel *lbl_MainWindow_aConnect;
    SequenceEdit *MainWindow_aConnect;
    QLabel *lbl_MainWindow_aRegister;
    QLabel *lbl_MainWindow_aFullScreen;
    SequenceEdit *MainWindow_aFullScreen;
    QLabel *lbl_MainWindow_aSettings;
    SequenceEdit *MainWindow_aRegister;
    QLabel *lbl_MainWindow_aCheckCardUpdates;
    SequenceEdit *MainWindow_aSettings;
    SequenceEdit *MainWindow_aCheckCardUpdates;
    SequenceEdit *MainWindow_aWatchReplay;
    SequenceEdit *MainWindow_aDisconnect;
    QLabel *lbl_MainWindow_aDisconnect;
    QLabel *lbl_MainWindow_aExit;
    SequenceEdit *MainWindow_aExit;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout;
    QLabel *lbl_TabDeckEditor_aAnalyzeDeck;
    SequenceEdit *TabDeckEditor_aAnalyzeDeck;
    QLabel *lbl_TabDeckEditor_aLoadDeckFromClipboard;
    SequenceEdit *TabDeckEditor_aLoadDeckFromClipboard;
    QLabel *lbl_TabDeckEditor_aClearFilterAll;
    SequenceEdit *TabDeckEditor_aClearFilterAll;
    QLabel *lbl_TabDeckEditor_aNewDeck;
    SequenceEdit *TabDeckEditor_aNewDeck;
    QLabel *lbl_TabDeckEditor_aClearFilterOne;
    SequenceEdit *TabDeckEditor_aClearFilterOne;
    QLabel *lbl_TabDeckEditor_aOpenCustomFolder;
    SequenceEdit *TabDeckEditor_aOpenCustomFolder;
    QLabel *lbl_TabDeckEditor_aClose;
    SequenceEdit *TabDeckEditor_aClose;
    QLabel *lbl_TabDeckEditor_aPrintDeck;
    SequenceEdit *TabDeckEditor_aPrintDeck;
    QLabel *lbl_TabDeckEditor_aManageSets;
    SequenceEdit *TabDeckEditor_aManageSets;
    QLabel *lbl_TabDeckEditor_aRemoveCard;
    SequenceEdit *TabDeckEditor_aRemoveCard;
    QLabel *lbl_TabDeckEditor_aEditTokens;
    SequenceEdit *TabDeckEditor_aEditTokens;
    QLabel *lbl_TabDeckEditor_aResetLayout;
    SequenceEdit *TabDeckEditor_aResetLayout;
    QLabel *lbl_TabDeckEditor_aIncrement;
    SequenceEdit *TabDeckEditor_aIncrement;
    QLabel *lbl_TabDeckEditor_aSaveDeck;
    SequenceEdit *TabDeckEditor_aSaveDeck;
    QLabel *lbl_TabDeckEditor_aExportDeckDecklist;
    SequenceEdit *TabDeckEditor_aExportDeckDecklist;
    QLabel *lbl_TabDeckEditor_aDecrement;
    SequenceEdit *TabDeckEditor_aDecrement;
    QLabel *lbl_TabDeckEditor_aSaveDeckAs;
    SequenceEdit *TabDeckEditor_aSaveDeckAs;
    QLabel *lbl_TabDeckEditor_aLoadDeck;
    SequenceEdit *TabDeckEditor_aLoadDeck;

    QLabel *lbl_TabDeckEditor_aSaveDeckToClipboard;
    SequenceEdit *TabDeckEditor_aSaveDeckToClipboard;
    QLabel *lbl_TabDeckEditor_aSaveDeckToClipboardRaw;
    SequenceEdit *TabDeckEditor_aSaveDeckToClipboardRaw;

    QGroupBox *groupBox_3;
    QGridLayout *counterLayout;
    QGroupBox *groupBox_4;
    QGridLayout *gridLayout_4;
    QLabel *lbl_abstractCounter_sSet;
    SequenceEdit *abstractCounter_aSet;
    QLabel *lbl_abstractCounter_aInc;
    SequenceEdit *abstractCounter_Inc;
    QLabel *lbl_abstractCounter_aDec;
    SequenceEdit *abstractCounter_aDec;
    QGroupBox *groupBox_5;
    QGridLayout *gridLayout_6;
    QLabel *lbl_Player_aSCRed;
    SequenceEdit *Player_aSCRed;
    QLabel *lbl_Player_aCCRed;
    SequenceEdit *Player_aCCRed;
    QLabel *lbl_Player_aRCRed;
    SequenceEdit *Player_aRCRed;
    QGroupBox *groupBox_6;
    QGridLayout *gridLayout_7;
    QLabel *lbl_Player_aSCGreen;
    SequenceEdit *Player_aSCGreen;
    QLabel *lbl_Player_aCCGreen;
    SequenceEdit *Player_aCCGreen;
    QLabel *lbl_Player_aRCGreen;
    SequenceEdit *Player_aRCGreen;
    QGroupBox *groupBox_7;
    QGridLayout *gridLayout_8;
    QLabel *lbl_Player_aSCYellow;
    SequenceEdit *Player_aSCYellow;
    QLabel *lbl_Player_aCCYellow;
    SequenceEdit *Player_aCCYellow;
    QLabel *lbl_Player_aRCYellow;
    SequenceEdit *Player_aRCYellow;

    QGroupBox *groupBox_counterStorm;
    QGridLayout *gridLayout_Storm;
    QLabel *lbl_Player_aSetCStorm;
    SequenceEdit *Player_aSetCStorm;
    QLabel *lbl_Player_aIncCStorm;
    SequenceEdit *Player_aIncCStorm;
    QLabel *lbl_Player_aDecCStorm;
    SequenceEdit *Player_aDecCStorm;

    QGroupBox *groupBox_counterW; // W Counter
    QGridLayout *gridLayout_W;
    QLabel *lbl_Player_aSetCW;
    SequenceEdit *Player_aSetCW;
    QLabel *lbl_Player_aIncCW;
    SequenceEdit *Player_aIncCW;
    QLabel *lbl_Player_aDecCW;
    SequenceEdit *Player_aDecCW;

    QGroupBox *groupBox_counterU; // U Counter
    QGridLayout *gridLayout_U;
    QLabel *lbl_Player_aSetCU;
    SequenceEdit *Player_aSetCU;
    QLabel *lbl_Player_aIncCU;
    SequenceEdit *Player_aIncCU;
    QLabel *lbl_Player_aDecCU;
    SequenceEdit *Player_aDecCU;

    QGroupBox *groupBox_counterB; // B Counter
    QGridLayout *gridLayout_B;
    QLabel *lbl_Player_aSetCB;
    SequenceEdit *Player_aSetCB;
    QLabel *lbl_Player_aIncCB;
    SequenceEdit *Player_aIncCB;
    QLabel *lbl_Player_aDecCB;
    SequenceEdit *Player_aDecCB;

    QGroupBox *groupBox_counterR; // R Counter
    QGridLayout *gridLayout_R;
    QLabel *lbl_Player_aSetCR;
    SequenceEdit *Player_aSetCR;
    QLabel *lbl_Player_aIncCR;
    SequenceEdit *Player_aIncCR;
    QLabel *lbl_Player_aDecCR;
    SequenceEdit *Player_aDecCR;

    QGroupBox *groupBox_counterG; // G Counter
    QGridLayout *gridLayout_G;
    QLabel *lbl_Player_aSetCG;
    SequenceEdit *Player_aSetCG;
    QLabel *lbl_Player_aIncCG;
    SequenceEdit *Player_aIncCG;
    QLabel *lbl_Player_aDecCG;
    SequenceEdit *Player_aDecCG;

    QGroupBox *groupBox_counterX; // X Counter
    QGridLayout *gridLayout_X;
    QLabel *lbl_Player_aSetCX;
    SequenceEdit *Player_aSetCX;
    QLabel *lbl_Player_aIncCX;
    SequenceEdit *Player_aIncCX;
    QLabel *lbl_Player_aDecCX;
    SequenceEdit *Player_aDecCX;

    QSpacerItem *verticalSpacer;
    QWidget *tab_2;
    QGridLayout *gridLayout_17;
    QGroupBox *groupBox_9;
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox_12;
    QGridLayout *gridLayout_12;
    SequenceEdit *Player_aDecPT;
    SequenceEdit *Player_aIncPT;
    QLabel *lbl_Player_aIncPT;
    QLabel *lbl_Player_aDecPT;
    SequenceEdit *Player_aSetPT;
    QLabel *lbl_Player_aSetPT;
    SequenceEdit *Player_aResetPT;
    QLabel *lbl_Player_aResetPT;
    QGroupBox *groupBox_11;
    QGridLayout *gridLayout_11;
    QLabel *lbl_Player_aDecT;
    SequenceEdit *Player_aDecT;
    QLabel *lbl_Player_aIncT;
    SequenceEdit *Player_aIncT;
    QGroupBox *groupBox_10;
    QGridLayout *gridLayout_10;
    QLabel *lbl_Player_aDecP;
    SequenceEdit *Player_aDecP;
    SequenceEdit *Player_aIncP;
    QLabel *lbl_Player_aIncP;
    QGroupBox *groupBox_8;
    QGridLayout *gridLayout_5;
    QLabel *lbl_TabGame_phase0;
    SequenceEdit *TabGame_phase0;
    QLabel *lbl_TabGame_phase1;
    SequenceEdit *TabGame_phase1;
    QLabel *lbl_TabGame_phase2;
    SequenceEdit *TabGame_phase2;
    QLabel *lbl_TabGame_phase3;
    SequenceEdit *TabGame_phase3;
    QLabel *lbl_TabGame_phase4;
    SequenceEdit *TabGame_phase4;
    QLabel *lbl_TabGame_phase5;
    SequenceEdit *TabGame_phase5;
    QLabel *lbl_TabGame_phase6;
    QLabel *lbl_TabGame_phase7;
    SequenceEdit *TabGame_phase6;
    QLabel *lbl_TabGame_phase8;
    SequenceEdit *TabGame_phase7;
    QLabel *lbl_TabGame_phase9;
    SequenceEdit *TabGame_phase8;
    QLabel *lbl_TabGame_phase10;
    SequenceEdit *TabGame_phase9;
    QLabel *lbl_TabGame_aNextPhase;
    SequenceEdit *TabGame_phase10;
    QLabel *lbl_TabGame_aNextTurn;
    SequenceEdit *TabGame_aNextPhase;
    QLabel *lbl_TabGame_aNextPhaseAction;
    SequenceEdit *TabGame_aNextPhaseAction;

    SequenceEdit *TabGame_aNextTurn;
    QGroupBox *groupBox_13;
    QGridLayout *gridLayout_13;
    QLabel *lbl_Player_aTap;
    SequenceEdit *Player_aTap;
    QLabel *lbl_Player_aUntapAll;
    SequenceEdit *Player_aUntapAll;
    QLabel *lbl_Player_aDoesntUntap;
    SequenceEdit *Player_aDoesntUntap;
    QLabel *lbl_Player_aFlip;
    SequenceEdit *Player_aFlip;
    QLabel *lbl_Player_aPeek;
    SequenceEdit *Player_aPeek;
    QLabel *lbl_Player_aPlay;
    SequenceEdit *Player_aPlay;
    QLabel *lbl_Player_aAttach;
    SequenceEdit *Player_aAttach;
    QLabel *lbl_Player_aUnattach;
    SequenceEdit *Player_aUnattach;
    QLabel *lbl_Player_aClone;
    SequenceEdit *Player_aClone;
    QLabel *lbl_Player_aCreateToken;
    SequenceEdit *Player_aCreateToken;
    QLabel *lbl_Player_aCreateRelatedTokens;
    SequenceEdit *Player_aCreateRelatedTokens;
    QLabel *lbl_Player_aCreateAnotherToken;
    SequenceEdit *Player_aCreateAnotherToken;
    QLabel *lbl_Player_aSetAnnotation;
    SequenceEdit *Player_aSetAnnotation;
    QSpacerItem *verticalSpacer_2;
    QWidget *tab_3;
    QGridLayout *gridLayout_20;
    QGroupBox *groupBox_gameplay;
    QGridLayout *gridLayout_gameplay;
    QLabel *lbl_Player_aDrawArrow;
    SequenceEdit *Player_aDrawArrow;
    QLabel *lbl_TabGame_aLeaveGame;
    SequenceEdit *TabGame_aLeaveGame;
    QLabel *lbl_TabGame_aRemoveLocalArrows;
    SequenceEdit *TabGame_aRemoveLocalArrows;
    QLabel *lbl_TabGame_aConcede;
    SequenceEdit *TabGame_aConcede;
    QLabel *lbl_Player_aRollDie;
    SequenceEdit *Player_aRollDie;
    QLabel *lbl_TabGame_aRotateViewCW;
    SequenceEdit *TabGame_aRotateViewCW;
    QLabel *lbl_Player_aShuffle;
    SequenceEdit *Player_aShuffle;
    QLabel *lbl_TabGame_aRotateViewCCW;
    SequenceEdit *TabGame_aRotateViewCCW;
    QGroupBox *groupBox_moveCard;
    QGridLayout *gridLayout_moveCard;
    QLabel *lbl_Player_aMoveToBottomLibrary;
    SequenceEdit *Player_aMoveToBottomLibrary;
    QLabel *lbl_Player_aMoveToTopLibrary;
    SequenceEdit *Player_aMoveToTopLibrary;
    QLabel *lbl_Player_aMoveToGraveyard;
    SequenceEdit *Player_aMoveToGraveyard;
    QLabel *lbl_Player_aMoveToExile;
    SequenceEdit *Player_aMoveToExile;
    QLabel *lbl_Player_aMoveToHand;
    SequenceEdit *Player_aMoveToHand;
    QLabel *lbl_Player_aMoveTopToPlayFaceDown;
    SequenceEdit *Player_aMoveTopToPlayFaceDown;
    QGroupBox *groupBox_view;
    QGridLayout *gridLayout_view;
    QLabel *lbl_Player_aViewGraveyard;
    SequenceEdit *Player_aViewGraveyard;
    QLabel *lbl_Player_aViewLibrary;
    SequenceEdit *Player_aViewLibrary;
    QLabel *lbl_Player_aViewTopCards;
    SequenceEdit *Player_aViewTopCards;
    QLabel *lbl_Player_aViewSideboard;
    SequenceEdit *Player_aViewSideboard;
    QLabel *lbl_Player_aViewRfg;
    SequenceEdit *Player_aViewRfg;
    QLabel *lbl_GameView_aCloseMostRecentZoneView;
    SequenceEdit *GameView_aCloseMostRecentZoneView;
    QGroupBox *groupBox_draw;
    QGridLayout *gridLayout_draw;
    QLabel *lbl_Player_aMulligan;
    SequenceEdit *Player_aMulligan;
    QLabel *lbl_Player_aDrawCard;
    SequenceEdit *Player_aDrawCard;
    QLabel *lbl_Player_aDrawCards;
    SequenceEdit *Player_aDrawCards;
    QLabel *lbl_Player_aUndoDraw;
    SequenceEdit *Player_aUndoDraw;
    QLabel *lbl_Player_aAlwaysRevealTopCard;
    SequenceEdit *Player_aAlwaysRevealTopCard;
    QGroupBox *groupBox_moveDeck;
    QGridLayout *gridLayout_moveDeck;
    QLabel *lbl_Player_aMoveTopCardToGraveyard;
    SequenceEdit *Player_aMoveTopCardToGraveyard;
    QLabel *lbl_Player_aMoveTopCardToExile;
    SequenceEdit *Player_aMoveTopCardToExile;
    QLabel *lbl_Player_aMoveTopCardsToGraveyard;
    SequenceEdit *Player_aMoveTopCardsToGraveyard;
    QLabel *lbl_Player_aMoveTopCardsToExile;
    SequenceEdit *Player_aMoveTopCardsToExile;
    QGroupBox *groupBox_gameLobby;
    QGridLayout *gridLayout_gameLobby;
    SequenceEdit *DeckViewContainer_loadRemoteButton;
    QLabel *lbl_DeckViewContainer_loadRemoteButton;
    SequenceEdit *DeckViewContainer_loadLocalButton;
    QLabel *lbl_DeckViewContainer_loadLocalButton;
    QSpacerItem *verticalSpacer_3;
    QWidget *tab_4;
    QLabel *faqLabel;
    QPushButton *btnResetAll;
    QPushButton *btnClearAll;

    void setupUi(QWidget *shortcutsTab)
    {
        if (shortcutsTab->objectName().isEmpty())
            shortcutsTab->setObjectName("shortcutsTab");
        shortcutsTab->resize(819, 477);
        gridLayout_9 = new QGridLayout(shortcutsTab);
        gridLayout_9->setObjectName("gridLayout_9");
        tabWidget = new QTabWidget(shortcutsTab);
        tabWidget->setObjectName("tabWidget");
        tab = new QWidget();
        tab->setObjectName("tab");
        gridLayout_3 = new QGridLayout(tab);
        gridLayout_3->setObjectName("gridLayout_3");
        groupBox = new QGroupBox(tab);
        groupBox->setObjectName("groupBox");
        gridLayout_2 = new QGridLayout(groupBox);
        gridLayout_2->setObjectName("gridLayout_2");
        lbl_MainWindow_aDeckEditor = new QLabel(groupBox);
        lbl_MainWindow_aDeckEditor->setObjectName("lbl_MainWindow_aDeckEditor");

        gridLayout_2->addWidget(lbl_MainWindow_aDeckEditor, 1, 0, 1, 1);

        MainWindow_aDeckEditor = new SequenceEdit("MainWindow/aDeckEditor", groupBox);
        MainWindow_aDeckEditor->setObjectName("MainWindow_aDeckEditor");

        gridLayout_2->addWidget(MainWindow_aDeckEditor, 1, 1, 1, 2);

        lbl_MainWindow_aSinglePlayer = new QLabel(groupBox);
        lbl_MainWindow_aSinglePlayer->setObjectName("lbl_MainWindow_aSinglePlayer");

        gridLayout_2->addWidget(lbl_MainWindow_aSinglePlayer, 2, 0, 1, 1);

        MainWindow_aSinglePlayer = new SequenceEdit("MainWindow/aSinglePlayer", groupBox);
        MainWindow_aSinglePlayer->setObjectName("MainWindow_aSinglePlayer");

        gridLayout_2->addWidget(MainWindow_aSinglePlayer, 2, 1, 1, 2);

        lbl_MainWindow_aWatchReplay = new QLabel(groupBox);
        lbl_MainWindow_aWatchReplay->setObjectName("lbl_MainWindow_aWatchReplay");

        gridLayout_2->addWidget(lbl_MainWindow_aWatchReplay, 4, 0, 1, 1);

        lbl_MainWindow_aConnect = new QLabel(groupBox);
        lbl_MainWindow_aConnect->setObjectName("lbl_MainWindow_aConnect");

        gridLayout_2->addWidget(lbl_MainWindow_aConnect, 6, 0, 1, 1);

        MainWindow_aConnect = new SequenceEdit("MainWindow/aConnect", groupBox);
        MainWindow_aConnect->setObjectName("MainWindow_aConnect");

        gridLayout_2->addWidget(MainWindow_aConnect, 6, 1, 1, 2);

        lbl_MainWindow_aRegister = new QLabel(groupBox);
        lbl_MainWindow_aRegister->setObjectName("lbl_MainWindow_aRegister");

        gridLayout_2->addWidget(lbl_MainWindow_aRegister, 10, 0, 1, 1);

        lbl_MainWindow_aFullScreen = new QLabel(groupBox);
        lbl_MainWindow_aFullScreen->setObjectName("lbl_MainWindow_aFullScreen");

        gridLayout_2->addWidget(lbl_MainWindow_aFullScreen, 9, 0, 1, 1);

        MainWindow_aFullScreen = new SequenceEdit("MainWindow/aFullScreen", groupBox);
        MainWindow_aFullScreen->setObjectName("MainWindow_aFullScreen");

        gridLayout_2->addWidget(MainWindow_aFullScreen, 9, 1, 1, 2);

        lbl_MainWindow_aSettings = new QLabel(groupBox);
        lbl_MainWindow_aSettings->setObjectName("lbl_MainWindow_aSettings");

        gridLayout_2->addWidget(lbl_MainWindow_aSettings, 11, 0, 1, 1);

        MainWindow_aRegister = new SequenceEdit("MainWindow/aRegister", groupBox);
        MainWindow_aRegister->setObjectName("MainWindow_aRegister");

        gridLayout_2->addWidget(MainWindow_aRegister, 10, 1, 1, 2);

        lbl_MainWindow_aCheckCardUpdates = new QLabel(groupBox);
        lbl_MainWindow_aCheckCardUpdates->setObjectName("lbl_MainWindow_aCheckCardUpdates");

        gridLayout_2->addWidget(lbl_MainWindow_aCheckCardUpdates, 0, 0, 1, 1);

        MainWindow_aSettings = new SequenceEdit("MainWindow/aSettings", groupBox);
        MainWindow_aSettings->setObjectName("MainWindow_aSettings");

        gridLayout_2->addWidget(MainWindow_aSettings, 11, 1, 1, 2);

        MainWindow_aCheckCardUpdates = new SequenceEdit("MainWindow/aCheckCardUpdates", groupBox);
        MainWindow_aCheckCardUpdates->setObjectName("MainWindow_aCheckCardUpdates");

        gridLayout_2->addWidget(MainWindow_aCheckCardUpdates, 0, 1, 1, 2);

        MainWindow_aWatchReplay = new SequenceEdit("MainWindow/aWatchReplay", groupBox);
        MainWindow_aWatchReplay->setObjectName("MainWindow_aWatchReplay");

        gridLayout_2->addWidget(MainWindow_aWatchReplay, 4, 1, 1, 2);

        MainWindow_aDisconnect = new SequenceEdit("MainWindow/aDisconnect", groupBox);
        MainWindow_aDisconnect->setObjectName("MainWindow_aDisconnect");

        gridLayout_2->addWidget(MainWindow_aDisconnect, 7, 1, 1, 2);

        lbl_MainWindow_aDisconnect = new QLabel(groupBox);
        lbl_MainWindow_aDisconnect->setObjectName("lbl_MainWindow_aDisconnect");

        gridLayout_2->addWidget(lbl_MainWindow_aDisconnect, 7, 0, 1, 1);

        lbl_MainWindow_aExit = new QLabel(groupBox);
        lbl_MainWindow_aExit->setObjectName("lbl_MainWindow_aExit");

        gridLayout_2->addWidget(lbl_MainWindow_aExit, 8, 0, 1, 1);

        MainWindow_aExit = new SequenceEdit("MainWindow/aExit", groupBox);
        MainWindow_aExit->setObjectName("MainWindow_aExit");

        gridLayout_2->addWidget(MainWindow_aExit, 8, 1, 1, 2);

        gridLayout_3->addWidget(groupBox, 0, 0, 1, 1);

        groupBox_2 = new QGroupBox(tab);
        groupBox_2->setObjectName("groupBox_2");
        gridLayout = new QGridLayout(groupBox_2);
        gridLayout->setObjectName("gridLayout");
        lbl_TabDeckEditor_aAnalyzeDeck = new QLabel(groupBox_2);
        lbl_TabDeckEditor_aAnalyzeDeck->setObjectName("lbl_TabDeckEditor_aAnalyzeDeck");

        gridLayout->addWidget(lbl_TabDeckEditor_aAnalyzeDeck, 0, 0, 1, 1);

        TabDeckEditor_aAnalyzeDeck = new SequenceEdit("TabDeckEditor/aAnalyzeDeck", groupBox_2);
        TabDeckEditor_aAnalyzeDeck->setObjectName("TabDeckEditor_aAnalyzeDeck");

        gridLayout->addWidget(TabDeckEditor_aAnalyzeDeck, 0, 1, 1, 1);

        lbl_TabDeckEditor_aLoadDeckFromClipboard = new QLabel(groupBox_2);
        lbl_TabDeckEditor_aLoadDeckFromClipboard->setObjectName("lbl_TabDeckEditor_aLoadDeckFromClipboard");

        gridLayout->addWidget(lbl_TabDeckEditor_aLoadDeckFromClipboard, 0, 2, 1, 1);

        TabDeckEditor_aLoadDeckFromClipboard = new SequenceEdit("TabDeckEditor/aLoadDeckFromClipboard", groupBox_2);
        TabDeckEditor_aLoadDeckFromClipboard->setObjectName("TabDeckEditor_aLoadDeckFromClipboard");

        gridLayout->addWidget(TabDeckEditor_aLoadDeckFromClipboard, 0, 3, 1, 1);

        lbl_TabDeckEditor_aClearFilterAll = new QLabel(groupBox_2);
        lbl_TabDeckEditor_aClearFilterAll->setObjectName("lbl_TabDeckEditor_aClearFilterAll");

        gridLayout->addWidget(lbl_TabDeckEditor_aClearFilterAll, 1, 0, 1, 1);

        TabDeckEditor_aClearFilterAll = new SequenceEdit("TabDeckEditor/aClearFilterAll", groupBox_2);
        TabDeckEditor_aClearFilterAll->setObjectName("TabDeckEditor_aClearFilterAll");

        gridLayout->addWidget(TabDeckEditor_aClearFilterAll, 1, 1, 1, 1);

        lbl_TabDeckEditor_aNewDeck = new QLabel(groupBox_2);
        lbl_TabDeckEditor_aNewDeck->setObjectName("lbl_TabDeckEditor_aNewDeck");

        gridLayout->addWidget(lbl_TabDeckEditor_aNewDeck, 1, 2, 1, 1);

        TabDeckEditor_aNewDeck = new SequenceEdit("TabDeckEditor/aNewDeck", groupBox_2);
        TabDeckEditor_aNewDeck->setObjectName("TabDeckEditor_aNewDeck");

        gridLayout->addWidget(TabDeckEditor_aNewDeck, 1, 3, 1, 1);

        lbl_TabDeckEditor_aClearFilterOne = new QLabel(groupBox_2);
        lbl_TabDeckEditor_aClearFilterOne->setObjectName("lbl_TabDeckEditor_aClearFilterOne");

        gridLayout->addWidget(lbl_TabDeckEditor_aClearFilterOne, 2, 0, 1, 1);

        TabDeckEditor_aClearFilterOne = new SequenceEdit("TabDeckEditor/aClearFilterOne", groupBox_2);
        TabDeckEditor_aClearFilterOne->setObjectName("TabDeckEditor_aClearFilterOne");

        gridLayout->addWidget(TabDeckEditor_aClearFilterOne, 2, 1, 1, 1);

        lbl_TabDeckEditor_aOpenCustomFolder = new QLabel(groupBox_2);
        lbl_TabDeckEditor_aOpenCustomFolder->setObjectName("lbl_TabDeckEditor_aOpenCustomFolder");

        gridLayout->addWidget(lbl_TabDeckEditor_aOpenCustomFolder, 2, 2, 1, 1);

        TabDeckEditor_aOpenCustomFolder = new SequenceEdit("TabDeckEditor/aOpenCustomFolder", groupBox_2);
        TabDeckEditor_aOpenCustomFolder->setObjectName("TabDeckEditor_aOpenCustomFolder");

        gridLayout->addWidget(TabDeckEditor_aOpenCustomFolder, 2, 3, 1, 1);

        lbl_TabDeckEditor_aClose = new QLabel(groupBox_2);
        lbl_TabDeckEditor_aClose->setObjectName("lbl_TabDeckEditor_aClose");

        gridLayout->addWidget(lbl_TabDeckEditor_aClose, 3, 0, 1, 1);

        TabDeckEditor_aClose = new SequenceEdit("TabDeckEditor/aClose", groupBox_2);
        TabDeckEditor_aClose->setObjectName("TabDeckEditor_aClose");

        gridLayout->addWidget(TabDeckEditor_aClose, 3, 1, 1, 1);

        lbl_TabDeckEditor_aPrintDeck = new QLabel(groupBox_2);
        lbl_TabDeckEditor_aPrintDeck->setObjectName("lbl_TabDeckEditor_aPrintDeck");

        gridLayout->addWidget(lbl_TabDeckEditor_aPrintDeck, 3, 2, 1, 1);

        TabDeckEditor_aPrintDeck = new SequenceEdit("TabDeckEditor/aPrintDeck", groupBox_2);
        TabDeckEditor_aPrintDeck->setObjectName("TabDeckEditor_aPrintDeck");

        gridLayout->addWidget(TabDeckEditor_aPrintDeck, 3, 3, 1, 1);

        lbl_TabDeckEditor_aManageSets = new QLabel(groupBox_2);
        lbl_TabDeckEditor_aManageSets->setObjectName("lbl_TabDeckEditor_aManageSets");

        gridLayout->addWidget(lbl_TabDeckEditor_aManageSets, 4, 0, 1, 1);

        TabDeckEditor_aManageSets = new SequenceEdit("TabDeckEditor/aManageSets", groupBox_2);
        TabDeckEditor_aManageSets->setObjectName("TabDeckEditor_aManageSets");

        gridLayout->addWidget(TabDeckEditor_aManageSets, 4, 1, 1, 1);

        lbl_TabDeckEditor_aRemoveCard = new QLabel(groupBox_2);
        lbl_TabDeckEditor_aRemoveCard->setObjectName("lbl_TabDeckEditor_aRemoveCard");

        gridLayout->addWidget(lbl_TabDeckEditor_aRemoveCard, 4, 2, 1, 1);

        TabDeckEditor_aRemoveCard = new SequenceEdit("TabDeckEditor/aRemoveCard", groupBox_2);
        TabDeckEditor_aRemoveCard->setObjectName("TabDeckEditor_aRemoveCard");

        gridLayout->addWidget(TabDeckEditor_aRemoveCard, 4, 3, 1, 1);

        lbl_TabDeckEditor_aEditTokens = new QLabel(groupBox_2);
        lbl_TabDeckEditor_aEditTokens->setObjectName("lbl_TabDeckEditor_aEditTokens");

        gridLayout->addWidget(lbl_TabDeckEditor_aEditTokens, 5, 0, 1, 1);

        TabDeckEditor_aEditTokens = new SequenceEdit("TabDeckEditor/aEditTokens", groupBox_2);
        TabDeckEditor_aEditTokens->setObjectName("TabDeckEditor_aEditTokens");

        gridLayout->addWidget(TabDeckEditor_aEditTokens, 5, 1, 1, 1);

        lbl_TabDeckEditor_aResetLayout = new QLabel(groupBox_2);
        lbl_TabDeckEditor_aResetLayout->setObjectName("lbl_TabDeckEditor_aResetLayout");

        gridLayout->addWidget(lbl_TabDeckEditor_aResetLayout, 5, 2, 1, 1);

        TabDeckEditor_aResetLayout = new SequenceEdit("TabDeckEditor/aResetLayout", groupBox_2);
        TabDeckEditor_aResetLayout->setObjectName("TabDeckEditor_aResetLayout");

        gridLayout->addWidget(TabDeckEditor_aResetLayout, 5, 3, 1, 1);

        lbl_TabDeckEditor_aIncrement = new QLabel(groupBox_2);
        lbl_TabDeckEditor_aIncrement->setObjectName("lbl_TabDeckEditor_aIncrement");

        gridLayout->addWidget(lbl_TabDeckEditor_aIncrement, 6, 0, 1, 1);

        TabDeckEditor_aIncrement = new SequenceEdit("TabDeckEditor/aIncrement", groupBox_2);
        TabDeckEditor_aIncrement->setObjectName("TabDeckEditor_aIncrement");

        gridLayout->addWidget(TabDeckEditor_aIncrement, 6, 1, 1, 1);

        lbl_TabDeckEditor_aSaveDeck = new QLabel(groupBox_2);
        lbl_TabDeckEditor_aSaveDeck->setObjectName("lbl_TabDeckEditor_aSaveDeck");

        gridLayout->addWidget(lbl_TabDeckEditor_aSaveDeck, 6, 2, 1, 1);

        TabDeckEditor_aSaveDeck = new SequenceEdit("TabDeckEditor/aSaveDeck", groupBox_2);
        TabDeckEditor_aSaveDeck->setObjectName("TabDeckEditor_aSaveDeck");

        gridLayout->addWidget(TabDeckEditor_aSaveDeck, 6, 3, 1, 1);

        lbl_TabDeckEditor_aDecrement = new QLabel(groupBox_2);
        lbl_TabDeckEditor_aDecrement->setObjectName("lbl_TabDeckEditor_aDecrement");

        gridLayout->addWidget(lbl_TabDeckEditor_aDecrement, 7, 0, 1, 1);

        TabDeckEditor_aDecrement = new SequenceEdit("TabDeckEditor/aDecrement", groupBox_2);
        TabDeckEditor_aDecrement->setObjectName("TabDeckEditor_aDecrement");

        gridLayout->addWidget(TabDeckEditor_aDecrement, 7, 1, 1, 1);

        lbl_TabDeckEditor_aSaveDeckAs = new QLabel(groupBox_2);
        lbl_TabDeckEditor_aSaveDeckAs->setObjectName("lbl_TabDeckEditor_aSaveDeckAs");

        gridLayout->addWidget(lbl_TabDeckEditor_aSaveDeckAs, 7, 2, 1, 1);

        TabDeckEditor_aSaveDeckAs = new SequenceEdit("TabDeckEditor/aSaveDeckAs", groupBox_2);
        TabDeckEditor_aSaveDeckAs->setObjectName("TabDeckEditor_aSaveDeckAs");

        gridLayout->addWidget(TabDeckEditor_aSaveDeckAs, 7, 3, 1, 1);

        lbl_TabDeckEditor_aLoadDeck = new QLabel(groupBox_2);
        lbl_TabDeckEditor_aLoadDeck->setObjectName("lbl_TabDeckEditor_aLoadDeck");

        gridLayout->addWidget(lbl_TabDeckEditor_aLoadDeck, 8, 0, 1, 1);

        TabDeckEditor_aLoadDeck = new SequenceEdit("TabDeckEditor/aLoadDeck", groupBox_2);
        TabDeckEditor_aLoadDeck->setObjectName("TabDeckEditor_aLoadDeck");

        gridLayout->addWidget(TabDeckEditor_aLoadDeck, 8, 1, 1, 1);

        lbl_TabDeckEditor_aSaveDeckToClipboard = new QLabel(groupBox_2);
        lbl_TabDeckEditor_aSaveDeckToClipboard->setObjectName("lbl_TabDeckEditor_aSaveDeckToClipboard");

        gridLayout->addWidget(lbl_TabDeckEditor_aSaveDeckToClipboard, 8, 2, 1, 1);

        lbl_TabDeckEditor_aSaveDeckToClipboardRaw = new QLabel(groupBox_2);
        lbl_TabDeckEditor_aSaveDeckToClipboard->setObjectName("lbl_TabDeckEditor_aSaveDeckToClipboardRaw");

        gridLayout->addWidget(lbl_TabDeckEditor_aSaveDeckToClipboardRaw, 9, 2, 1, 1);

        TabDeckEditor_aSaveDeckToClipboard = new SequenceEdit("TabDeckEditor/aSaveDeckToClipboard", groupBox_2);
        TabDeckEditor_aSaveDeckToClipboard->setObjectName("TabDeckEditor_aSaveDeckToClipboard");

        gridLayout->addWidget(TabDeckEditor_aSaveDeckToClipboard, 8, 3, 1, 1);

        TabDeckEditor_aSaveDeckToClipboardRaw = new SequenceEdit("TabDeckEditor/aSaveDeckToClipboardRaw", groupBox_2);
        TabDeckEditor_aSaveDeckToClipboardRaw->setObjectName("TabDeckEditor_aSaveDeckToClipboardRaw");

        gridLayout->addWidget(TabDeckEditor_aSaveDeckToClipboardRaw, 9, 3, 1, 1);

        lbl_TabDeckEditor_aExportDeckDecklist = new QLabel(groupBox_2);
        lbl_TabDeckEditor_aExportDeckDecklist->setObjectName("lbl_TabDeckEditor_aExportDeckDecklist");

        // TODO: MOVE THIS SHIT
        gridLayout->addWidget(lbl_TabDeckEditor_aExportDeckDecklist, 9, 0, 1, 1);

        TabDeckEditor_aExportDeckDecklist = new SequenceEdit("TabDeckEditor/aExportDeckDecklist", groupBox_2);
        TabDeckEditor_aExportDeckDecklist->setObjectName("TabDeckEditor_aExportDeckDecklist");

        gridLayout->addWidget(TabDeckEditor_aExportDeckDecklist, 9, 1, 1, 1);

        gridLayout_3->addWidget(groupBox_2, 0, 1, 1, 1);

        groupBox_3 = new QGroupBox(tab);
        groupBox_3->setObjectName("groupBox_3");
        counterLayout = new QGridLayout(groupBox_3);
        counterLayout->setObjectName("counterLayout");
        groupBox_4 = new QGroupBox(groupBox_3);
        groupBox_4->setObjectName("groupBox_4");
        gridLayout_4 = new QGridLayout(groupBox_4);
        gridLayout_4->setObjectName("gridLayout_4");
        lbl_abstractCounter_sSet = new QLabel(groupBox_4);
        lbl_abstractCounter_sSet->setObjectName("lbl_abstractCounter_sSet");

        gridLayout_4->addWidget(lbl_abstractCounter_sSet, 0, 0, 1, 1);

        abstractCounter_aSet = new SequenceEdit("Player/aSet", groupBox_4);
        abstractCounter_aSet->setObjectName("abstractCounter_aSet");

        gridLayout_4->addWidget(abstractCounter_aSet, 0, 1, 1, 1);

        lbl_abstractCounter_aInc = new QLabel(groupBox_4);
        lbl_abstractCounter_aInc->setObjectName("lbl_abstractCounter_aInc");

        gridLayout_4->addWidget(lbl_abstractCounter_aInc, 1, 0, 1, 1);

        abstractCounter_Inc = new SequenceEdit("Player/aInc", groupBox_4);
        abstractCounter_Inc->setObjectName("abstractCounter_Inc");

        gridLayout_4->addWidget(abstractCounter_Inc, 1, 1, 1, 1);

        lbl_abstractCounter_aDec = new QLabel(groupBox_4);
        lbl_abstractCounter_aDec->setObjectName("lbl_abstractCounter_aDec");

        gridLayout_4->addWidget(lbl_abstractCounter_aDec, 2, 0, 1, 1);

        abstractCounter_aDec = new SequenceEdit("Player/aDec", groupBox_4);
        abstractCounter_aDec->setObjectName("abstractCounter_aDec");

        gridLayout_4->addWidget(abstractCounter_aDec, 2, 1, 1, 1);

        counterLayout->addWidget(groupBox_4, 0, 0, 1, 1);

        groupBox_5 = new QGroupBox(groupBox_3);
        groupBox_5->setObjectName("groupBox_5");
        gridLayout_6 = new QGridLayout(groupBox_5);
        gridLayout_6->setObjectName("gridLayout_6");
        lbl_Player_aSCRed = new QLabel(groupBox_5);
        lbl_Player_aSCRed->setObjectName("lbl_Player_aSCRed");

        gridLayout_6->addWidget(lbl_Player_aSCRed, 0, 0, 1, 1);

        Player_aSCRed = new SequenceEdit("Player/aSCRed", groupBox_5);
        Player_aSCRed->setObjectName("Player_aSCRed");

        gridLayout_6->addWidget(Player_aSCRed, 0, 1, 1, 1);

        lbl_Player_aCCRed = new QLabel(groupBox_5);
        lbl_Player_aCCRed->setObjectName("lbl_Player_aCCRed");

        gridLayout_6->addWidget(lbl_Player_aCCRed, 1, 0, 1, 1);

        Player_aCCRed = new SequenceEdit("Player/aCCRed", groupBox_5);
        Player_aCCRed->setObjectName("Player_aCCRed");

        gridLayout_6->addWidget(Player_aCCRed, 1, 1, 1, 1);

        lbl_Player_aRCRed = new QLabel(groupBox_5);
        lbl_Player_aRCRed->setObjectName("lbl_Player_aRCRed");

        gridLayout_6->addWidget(lbl_Player_aRCRed, 2, 0, 1, 1);

        Player_aRCRed = new SequenceEdit("Player/aRCRed", groupBox_5);
        Player_aRCRed->setObjectName("Player_aRCRed");

        gridLayout_6->addWidget(Player_aRCRed, 2, 1, 1, 1);

        counterLayout->addWidget(groupBox_5, 0, 1, 1, 1);

        groupBox_6 = new QGroupBox(groupBox_3);
        groupBox_6->setObjectName("groupBox_6");
        gridLayout_7 = new QGridLayout(groupBox_6);
        gridLayout_7->setObjectName("gridLayout_7");
        lbl_Player_aSCGreen = new QLabel(groupBox_6);
        lbl_Player_aSCGreen->setObjectName("lbl_Player_aSCGreen");

        gridLayout_7->addWidget(lbl_Player_aSCGreen, 0, 0, 1, 1);

        Player_aSCGreen = new SequenceEdit("Player/aSCGreen", groupBox_6);
        Player_aSCGreen->setObjectName("Player_aSCGreen");

        gridLayout_7->addWidget(Player_aSCGreen, 0, 1, 1, 1);

        lbl_Player_aCCGreen = new QLabel(groupBox_6);
        lbl_Player_aCCGreen->setObjectName("lbl_Player_aCCGreen");

        gridLayout_7->addWidget(lbl_Player_aCCGreen, 1, 0, 1, 1);

        Player_aCCGreen = new SequenceEdit("Player/aCCGreen", groupBox_6);
        Player_aCCGreen->setObjectName("Player_aCCGreen");

        gridLayout_7->addWidget(Player_aCCGreen, 1, 1, 1, 1);

        lbl_Player_aRCGreen = new QLabel(groupBox_6);
        lbl_Player_aRCGreen->setObjectName("lbl_Player_aRCGreen");

        gridLayout_7->addWidget(lbl_Player_aRCGreen, 2, 0, 1, 1);

        Player_aRCGreen = new SequenceEdit("Player/aRCGreen", groupBox_6);
        Player_aRCGreen->setObjectName("Player_aRCGreen");

        gridLayout_7->addWidget(Player_aRCGreen, 2, 1, 1, 1);

        counterLayout->addWidget(groupBox_6, 0, 2, 1, 1);

        groupBox_7 = new QGroupBox(groupBox_3);
        groupBox_7->setObjectName("groupBox_7");
        gridLayout_8 = new QGridLayout(groupBox_7);
        gridLayout_8->setObjectName("gridLayout_8");
        lbl_Player_aSCYellow = new QLabel(groupBox_7);
        lbl_Player_aSCYellow->setObjectName("lbl_Player_aSCYellow");

        gridLayout_8->addWidget(lbl_Player_aSCYellow, 0, 0, 1, 1);

        Player_aSCYellow = new SequenceEdit("Player/aSCYellow", groupBox_7);
        Player_aSCYellow->setObjectName("Player_aSCYellow");

        gridLayout_8->addWidget(Player_aSCYellow, 0, 1, 1, 1);

        lbl_Player_aCCYellow = new QLabel(groupBox_7);
        lbl_Player_aCCYellow->setObjectName("lbl_Player_aCCYellow");

        gridLayout_8->addWidget(lbl_Player_aCCYellow, 1, 0, 1, 1);

        Player_aCCYellow = new SequenceEdit("Player/aCCYellow", groupBox_7);
        Player_aCCYellow->setObjectName("Player_aCCYellow");

        gridLayout_8->addWidget(Player_aCCYellow, 1, 1, 1, 1);

        lbl_Player_aRCYellow = new QLabel(groupBox_7);
        lbl_Player_aRCYellow->setObjectName("lbl_Player_aRCYellow");

        gridLayout_8->addWidget(lbl_Player_aRCYellow, 2, 0, 1, 1);

        Player_aRCYellow = new SequenceEdit("Player/aRCYellow", groupBox_7);
        Player_aRCYellow->setObjectName("Player_aRCYellow");

        gridLayout_8->addWidget(Player_aRCYellow, 2, 1, 1, 1);

        counterLayout->addWidget(groupBox_7, 0, 3, 1, 1);

        groupBox_counterStorm = new QGroupBox(groupBox_3);
        groupBox_counterStorm->setObjectName("groupBox_counterStorm");
        gridLayout_Storm = new QGridLayout(groupBox_counterStorm);
        gridLayout_Storm->setObjectName("gridLayout_Storm");
        lbl_Player_aSetCStorm = new QLabel(groupBox_counterStorm);
        lbl_Player_aSetCStorm->setObjectName("lbl_Player_aSetCStorm");
        gridLayout_Storm->addWidget(lbl_Player_aSetCStorm, 0, 0, 1, 1);
        Player_aSetCStorm = new SequenceEdit("Player/aSetCounter_storm", groupBox_counterStorm);
        Player_aSetCStorm->setObjectName("Player_aSetCStorm");
        gridLayout_Storm->addWidget(Player_aSetCStorm, 0, 1, 1, 1);

        lbl_Player_aIncCStorm = new QLabel(groupBox_counterStorm);
        lbl_Player_aIncCStorm->setObjectName("lbl_Player_aIncCStorm");
        gridLayout_Storm->addWidget(lbl_Player_aIncCStorm, 1, 0, 1, 1);
        Player_aIncCStorm = new SequenceEdit("Player/aIncCounter_storm", groupBox_counterStorm);
        Player_aIncCStorm->setObjectName("Player_aIncCStorm");
        gridLayout_Storm->addWidget(Player_aIncCStorm, 1, 1, 1, 1);

        lbl_Player_aDecCStorm = new QLabel(groupBox_counterStorm);
        lbl_Player_aDecCStorm->setObjectName("lbl_Player_aDecCStorm");
        gridLayout_Storm->addWidget(lbl_Player_aDecCStorm, 2, 0, 1, 1);
        Player_aDecCStorm = new SequenceEdit("Player/aDecCounter_storm", groupBox_counterStorm);
        Player_aDecCStorm->setObjectName("Player_aDecCStorm");
        gridLayout_Storm->addWidget(Player_aDecCStorm, 2, 1, 1, 1);

        counterLayout->addWidget(groupBox_counterStorm, 1, 0, 1, 1);

        groupBox_counterW = new QGroupBox(groupBox_3);
        groupBox_counterW->setObjectName("groupBox_counterW");
        gridLayout_W = new QGridLayout(groupBox_counterW);
        gridLayout_W->setObjectName("gridLayout_W");
        lbl_Player_aSetCW = new QLabel(groupBox_counterW);
        lbl_Player_aSetCW->setObjectName("lbl_Player_aSetCW");
        gridLayout_W->addWidget(lbl_Player_aSetCW, 0, 0, 1, 1);
        Player_aSetCW = new SequenceEdit("Player/aSetCounter_w", groupBox_counterW);
        Player_aSetCW->setObjectName("Player_aSetCW");
        gridLayout_W->addWidget(Player_aSetCW, 0, 1, 1, 1);

        lbl_Player_aIncCW = new QLabel(groupBox_counterW);
        lbl_Player_aIncCW->setObjectName("lbl_Player_aIncCW");
        gridLayout_W->addWidget(lbl_Player_aIncCW, 1, 0, 1, 1);
        Player_aIncCW = new SequenceEdit("Player/aIncCounter_w", groupBox_counterW);
        Player_aIncCW->setObjectName("Player_aIncCW");
        gridLayout_W->addWidget(Player_aIncCW, 1, 1, 1, 1);

        lbl_Player_aDecCW = new QLabel(groupBox_counterW);
        lbl_Player_aDecCW->setObjectName("lbl_Player_aDecCW");
        gridLayout_W->addWidget(lbl_Player_aDecCW, 2, 0, 1, 1);
        Player_aDecCW = new SequenceEdit("Player/aDecCounter_w", groupBox_counterW);
        Player_aDecCW->setObjectName("Player_aDecCW");
        gridLayout_W->addWidget(Player_aDecCW, 2, 1, 1, 1);

        counterLayout->addWidget(groupBox_counterW, 1, 1, 1, 1);

        groupBox_counterU = new QGroupBox(groupBox_3);
        groupBox_counterU->setObjectName("groupBox_counterU");
        gridLayout_U = new QGridLayout(groupBox_counterU);
        gridLayout_U->setObjectName("gridLayout_U");
        lbl_Player_aSetCU = new QLabel(groupBox_counterU);
        lbl_Player_aSetCU->setObjectName("lbl_Player_aSetCU");
        gridLayout_U->addWidget(lbl_Player_aSetCU, 0, 0, 1, 1);
        Player_aSetCU = new SequenceEdit("Player/aSetCounter_u", groupBox_counterU);
        Player_aSetCU->setObjectName("Player_aSetCU");
        gridLayout_U->addWidget(Player_aSetCU, 0, 1, 1, 1);

        lbl_Player_aIncCU = new QLabel(groupBox_counterU);
        lbl_Player_aIncCU->setObjectName("lbl_Player_aIncCU");
        gridLayout_U->addWidget(lbl_Player_aIncCU, 1, 0, 1, 1);
        Player_aIncCU = new SequenceEdit("Player/aIncCounter_u", groupBox_counterU);
        Player_aIncCU->setObjectName("Player_aIncCU");
        gridLayout_U->addWidget(Player_aIncCU, 1, 1, 1, 1);

        lbl_Player_aDecCU = new QLabel(groupBox_counterU);
        lbl_Player_aDecCU->setObjectName("lbl_Player_aDecCU");
        gridLayout_U->addWidget(lbl_Player_aDecCU, 2, 0, 1, 1);
        Player_aDecCU = new SequenceEdit("Player/aDecCounter_u", groupBox_counterU);
        Player_aDecCU->setObjectName("Player_aDecCU");
        gridLayout_U->addWidget(Player_aDecCU, 2, 1, 1, 1);

        counterLayout->addWidget(groupBox_counterU, 1, 2, 1, 1);

        groupBox_counterB = new QGroupBox(groupBox_3);
        groupBox_counterB->setObjectName("groupBox_counterB");
        gridLayout_B = new QGridLayout(groupBox_counterB);
        gridLayout_B->setObjectName("gridLayout_B");
        lbl_Player_aSetCB = new QLabel(groupBox_counterB);
        lbl_Player_aSetCB->setObjectName("lbl_Player_aSetCB");
        gridLayout_B->addWidget(lbl_Player_aSetCB, 0, 0, 1, 1);
        Player_aSetCB = new SequenceEdit("Player/aSetCounter_b", groupBox_counterB);
        Player_aSetCB->setObjectName("Player_aSetCB");
        gridLayout_B->addWidget(Player_aSetCB, 0, 1, 1, 1);

        lbl_Player_aIncCB = new QLabel(groupBox_counterB);
        lbl_Player_aIncCB->setObjectName("lbl_Player_aIncCB");
        gridLayout_B->addWidget(lbl_Player_aIncCB, 1, 0, 1, 1);
        Player_aIncCB = new SequenceEdit("Player/aIncCounter_b", groupBox_counterB);
        Player_aIncCB->setObjectName("Player_aIncCB");
        gridLayout_B->addWidget(Player_aIncCB, 1, 1, 1, 1);

        lbl_Player_aDecCB = new QLabel(groupBox_counterB);
        lbl_Player_aDecCB->setObjectName("lbl_Player_aDecCB");
        gridLayout_B->addWidget(lbl_Player_aDecCB, 2, 0, 1, 1);
        Player_aDecCB = new SequenceEdit("Player/aDecCounter_b", groupBox_counterB);
        Player_aDecCB->setObjectName("Player_aDecCB");
        gridLayout_B->addWidget(Player_aDecCB, 2, 1, 1, 1);

        counterLayout->addWidget(groupBox_counterB, 1, 3, 1, 1);

        groupBox_counterR = new QGroupBox(groupBox_3);
        groupBox_counterR->setObjectName("groupBox_counterR");
        gridLayout_R = new QGridLayout(groupBox_counterR);
        gridLayout_R->setObjectName("gridLayout_R");
        lbl_Player_aSetCR = new QLabel(groupBox_counterR);
        lbl_Player_aSetCR->setObjectName("lbl_Player_aSetCR");
        gridLayout_R->addWidget(lbl_Player_aSetCR, 0, 0, 1, 1);
        Player_aSetCR = new SequenceEdit("Player/aSetCounter_r", groupBox_counterR);
        Player_aSetCR->setObjectName("Player_aSetCR");
        gridLayout_R->addWidget(Player_aSetCR, 0, 1, 1, 1);

        lbl_Player_aIncCR = new QLabel(groupBox_counterR);
        lbl_Player_aIncCR->setObjectName("lbl_Player_aIncCR");
        gridLayout_R->addWidget(lbl_Player_aIncCR, 1, 0, 1, 1);
        Player_aIncCR = new SequenceEdit("Player/aIncCounter_r", groupBox_counterR);
        Player_aIncCR->setObjectName("Player_aIncCR");
        gridLayout_R->addWidget(Player_aIncCR, 1, 1, 1, 1);

        lbl_Player_aDecCR = new QLabel(groupBox_counterR);
        lbl_Player_aDecCR->setObjectName("lbl_Player_aDecCR");
        gridLayout_R->addWidget(lbl_Player_aDecCR, 2, 0, 1, 1);
        Player_aDecCR = new SequenceEdit("Player/aDecCounter_r", groupBox_counterR);
        Player_aDecCR->setObjectName("Player_aDecCR");
        gridLayout_R->addWidget(Player_aDecCR, 2, 1, 1, 1);

        counterLayout->addWidget(groupBox_counterR, 2, 0, 1, 1);

        groupBox_counterG = new QGroupBox(groupBox_3);
        groupBox_counterG->setObjectName("groupBox_counterG");
        gridLayout_G = new QGridLayout(groupBox_counterG);
        gridLayout_G->setObjectName("gridLayout_G");
        lbl_Player_aSetCG = new QLabel(groupBox_counterG);
        lbl_Player_aSetCG->setObjectName("lbl_Player_aSetCG");
        gridLayout_G->addWidget(lbl_Player_aSetCG, 0, 0, 1, 1);
        Player_aSetCG = new SequenceEdit("Player/aSetCounter_g", groupBox_counterG);
        Player_aSetCG->setObjectName("Player_aSetCG");
        gridLayout_G->addWidget(Player_aSetCG, 0, 1, 1, 1);

        lbl_Player_aIncCG = new QLabel(groupBox_counterG);
        lbl_Player_aIncCG->setObjectName("lbl_Player_aIncCG");
        gridLayout_G->addWidget(lbl_Player_aIncCG, 1, 0, 1, 1);
        Player_aIncCG = new SequenceEdit("Player/aIncCounter_g", groupBox_counterG);
        Player_aIncCG->setObjectName("Player_aIncCG");
        gridLayout_G->addWidget(Player_aIncCG, 1, 1, 1, 1);

        lbl_Player_aDecCG = new QLabel(groupBox_counterG);
        lbl_Player_aDecCG->setObjectName("lbl_Player_aDecCG");
        gridLayout_G->addWidget(lbl_Player_aDecCG, 2, 0, 1, 1);
        Player_aDecCG = new SequenceEdit("Player/aDecCounter_g", groupBox_counterG);
        Player_aDecCG->setObjectName("Player_aDecCG");
        gridLayout_G->addWidget(Player_aDecCG, 2, 1, 1, 1);

        counterLayout->addWidget(groupBox_counterG, 2, 1, 1, 1);

        groupBox_counterX = new QGroupBox(groupBox_3);
        groupBox_counterX->setObjectName("groupBox_counterX");
        gridLayout_X = new QGridLayout(groupBox_counterX);
        gridLayout_X->setObjectName("gridLayout_X");
        lbl_Player_aSetCX = new QLabel(groupBox_counterX);
        lbl_Player_aSetCX->setObjectName("lbl_Player_aSetCX");
        gridLayout_X->addWidget(lbl_Player_aSetCX, 0, 0, 1, 1);
        Player_aSetCX = new SequenceEdit("Player/aSetCounter_x", groupBox_counterX);
        Player_aSetCX->setObjectName("Player_aSetCX");
        gridLayout_X->addWidget(Player_aSetCX, 0, 1, 1, 1);

        lbl_Player_aIncCX = new QLabel(groupBox_counterX);
        lbl_Player_aIncCX->setObjectName("lbl_Player_aIncCX");
        gridLayout_X->addWidget(lbl_Player_aIncCX, 1, 0, 1, 1);
        Player_aIncCX = new SequenceEdit("Player/aIncCounter_x", groupBox_counterX);
        Player_aIncCX->setObjectName("Player_aIncCX");
        gridLayout_X->addWidget(Player_aIncCX, 1, 1, 1, 1);

        lbl_Player_aDecCX = new QLabel(groupBox_counterX);
        lbl_Player_aDecCX->setObjectName("lbl_Player_aDecCX");
        gridLayout_X->addWidget(lbl_Player_aDecCX, 2, 0, 1, 1);
        Player_aDecCX = new SequenceEdit("Player/aDecCounter_x", groupBox_counterX);
        Player_aDecCX->setObjectName("Player_aDecCX");
        gridLayout_X->addWidget(Player_aDecCX, 2, 1, 1, 1);

        counterLayout->addWidget(groupBox_counterX, 2, 2, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_3->addItem(verticalSpacer, 2, 0, 1, 1);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName("tab_2");
        gridLayout_17 = new QGridLayout(tab_2);
        gridLayout_17->setObjectName("gridLayout_17");
        groupBox_9 = new QGroupBox(tab_2);
        groupBox_9->setObjectName("groupBox_9");
        verticalLayout = new QVBoxLayout(groupBox_9);
        verticalLayout->setObjectName("verticalLayout");
        groupBox_12 = new QGroupBox(groupBox_9);
        groupBox_12->setObjectName("groupBox_12");
        gridLayout_12 = new QGridLayout(groupBox_12);
        gridLayout_12->setObjectName("gridLayout_12");
        Player_aDecPT = new SequenceEdit("Player/aDecPT", groupBox_12);
        Player_aDecPT->setObjectName("Player_aDecPT");

        gridLayout_12->addWidget(Player_aDecPT, 3, 1, 1, 1);

        Player_aIncPT = new SequenceEdit("Player/aIncPT", groupBox_12);
        Player_aIncPT->setObjectName("Player_aIncPT");

        gridLayout_12->addWidget(Player_aIncPT, 2, 1, 1, 1);

        lbl_Player_aIncPT = new QLabel(groupBox_12);
        lbl_Player_aIncPT->setObjectName("lbl_Player_aIncPT");

        gridLayout_12->addWidget(lbl_Player_aIncPT, 2, 0, 1, 1);

        lbl_Player_aDecPT = new QLabel(groupBox_12);
        lbl_Player_aDecPT->setObjectName("lbl_Player_aDecPT");

        gridLayout_12->addWidget(lbl_Player_aDecPT, 3, 0, 1, 1);

        Player_aSetPT = new SequenceEdit("Player/aSetPT", groupBox_12);
        Player_aSetPT->setObjectName("Player_aSetPT");

        gridLayout_12->addWidget(Player_aSetPT, 0, 1, 1, 1);

        lbl_Player_aSetPT = new QLabel(groupBox_12);
        lbl_Player_aSetPT->setObjectName("lbl_Player_aSetPT");

        gridLayout_12->addWidget(lbl_Player_aSetPT, 0, 0, 1, 1);

        Player_aResetPT = new SequenceEdit("Player/aResetPT", groupBox_12);
        Player_aResetPT->setObjectName("Player_aResetPT");

        gridLayout_12->addWidget(Player_aResetPT, 1, 1, 1, 1);

        lbl_Player_aResetPT = new QLabel(groupBox_12);
        lbl_Player_aResetPT->setObjectName("lbl_Player_aResetPT");

        gridLayout_12->addWidget(lbl_Player_aResetPT, 1, 0, 1, 1);

        verticalLayout->addWidget(groupBox_12);

        groupBox_11 = new QGroupBox(groupBox_9);
        groupBox_11->setObjectName("groupBox_11");
        gridLayout_11 = new QGridLayout(groupBox_11);
        gridLayout_11->setObjectName("gridLayout_11");
        lbl_Player_aDecT = new QLabel(groupBox_11);
        lbl_Player_aDecT->setObjectName("lbl_Player_aDecT");

        gridLayout_11->addWidget(lbl_Player_aDecT, 1, 0, 1, 1);

        Player_aDecT = new SequenceEdit("Player/aDecT", groupBox_11);
        Player_aDecT->setObjectName("Player_aDecT");

        gridLayout_11->addWidget(Player_aDecT, 1, 1, 1, 1);

        lbl_Player_aIncT = new QLabel(groupBox_11);
        lbl_Player_aIncT->setObjectName("lbl_Player_aIncT");

        gridLayout_11->addWidget(lbl_Player_aIncT, 0, 0, 1, 1);

        Player_aIncT = new SequenceEdit("Player/aIncT", groupBox_11);
        Player_aIncT->setObjectName("Player_aIncT");

        gridLayout_11->addWidget(Player_aIncT, 0, 1, 1, 1);

        verticalLayout->addWidget(groupBox_11);

        groupBox_10 = new QGroupBox(groupBox_9);
        groupBox_10->setObjectName("groupBox_10");
        gridLayout_10 = new QGridLayout(groupBox_10);
        gridLayout_10->setObjectName("gridLayout_10");
        lbl_Player_aDecP = new QLabel(groupBox_10);
        lbl_Player_aDecP->setObjectName("lbl_Player_aDecP");

        gridLayout_10->addWidget(lbl_Player_aDecP, 1, 0, 1, 1);

        Player_aDecP = new SequenceEdit("Player/aDecP", groupBox_10);
        Player_aDecP->setObjectName("Player_aDecP");

        gridLayout_10->addWidget(Player_aDecP, 1, 1, 1, 1);

        Player_aIncP = new SequenceEdit("Player/aIncP", groupBox_10);
        Player_aIncP->setObjectName("Player_aIncP");

        gridLayout_10->addWidget(Player_aIncP, 0, 1, 1, 1);

        lbl_Player_aIncP = new QLabel(groupBox_10);
        lbl_Player_aIncP->setObjectName("lbl_Player_aIncP");

        gridLayout_10->addWidget(lbl_Player_aIncP, 0, 0, 1, 1);

        verticalLayout->addWidget(groupBox_10);

        gridLayout_17->addWidget(groupBox_9, 0, 1, 1, 1);

        groupBox_8 = new QGroupBox(tab_2);
        groupBox_8->setObjectName("groupBox_8");
        gridLayout_5 = new QGridLayout(groupBox_8);
        gridLayout_5->setObjectName("gridLayout_5");
        lbl_TabGame_phase0 = new QLabel(groupBox_8);
        lbl_TabGame_phase0->setObjectName("lbl_TabGame_phase0");

        gridLayout_5->addWidget(lbl_TabGame_phase0, 0, 0, 1, 1);

        TabGame_phase0 = new SequenceEdit("Player/phase0", groupBox_8);
        TabGame_phase0->setObjectName("TabGame_phase0");

        gridLayout_5->addWidget(TabGame_phase0, 0, 1, 1, 1);

        lbl_TabGame_phase1 = new QLabel(groupBox_8);
        lbl_TabGame_phase1->setObjectName("lbl_TabGame_phase1");

        gridLayout_5->addWidget(lbl_TabGame_phase1, 1, 0, 1, 1);

        TabGame_phase1 = new SequenceEdit("Player/phase1", groupBox_8);
        TabGame_phase1->setObjectName("TabGame_phase1");

        gridLayout_5->addWidget(TabGame_phase1, 1, 1, 1, 1);

        lbl_TabGame_phase2 = new QLabel(groupBox_8);
        lbl_TabGame_phase2->setObjectName("lbl_TabGame_phase2");

        gridLayout_5->addWidget(lbl_TabGame_phase2, 2, 0, 1, 1);

        TabGame_phase2 = new SequenceEdit("Player/phase2", groupBox_8);
        TabGame_phase2->setObjectName("TabGame_phase2");

        gridLayout_5->addWidget(TabGame_phase2, 2, 1, 1, 1);

        lbl_TabGame_phase3 = new QLabel(groupBox_8);
        lbl_TabGame_phase3->setObjectName("lbl_TabGame_phase3");

        gridLayout_5->addWidget(lbl_TabGame_phase3, 3, 0, 1, 1);

        TabGame_phase3 = new SequenceEdit("Player/phase3", groupBox_8);
        TabGame_phase3->setObjectName("TabGame_phase3");

        gridLayout_5->addWidget(TabGame_phase3, 3, 1, 1, 1);

        lbl_TabGame_phase4 = new QLabel(groupBox_8);
        lbl_TabGame_phase4->setObjectName("lbl_TabGame_phase4");

        gridLayout_5->addWidget(lbl_TabGame_phase4, 4, 0, 1, 1);

        TabGame_phase4 = new SequenceEdit("Player/phase4", groupBox_8);
        TabGame_phase4->setObjectName("TabGame_phase4");

        gridLayout_5->addWidget(TabGame_phase4, 4, 1, 1, 1);

        lbl_TabGame_phase5 = new QLabel(groupBox_8);
        lbl_TabGame_phase5->setObjectName("lbl_TabGame_phase5");

        gridLayout_5->addWidget(lbl_TabGame_phase5, 5, 0, 1, 1);

        TabGame_phase5 = new SequenceEdit("Player/phase5", groupBox_8);
        TabGame_phase5->setObjectName("TabGame_phase5");

        gridLayout_5->addWidget(TabGame_phase5, 5, 1, 1, 1);

        lbl_TabGame_phase6 = new QLabel(groupBox_8);
        lbl_TabGame_phase6->setObjectName("lbl_TabGame_phase6");

        gridLayout_5->addWidget(lbl_TabGame_phase6, 6, 0, 1, 1);

        lbl_TabGame_phase7 = new QLabel(groupBox_8);
        lbl_TabGame_phase7->setObjectName("lbl_TabGame_phase7");

        gridLayout_5->addWidget(lbl_TabGame_phase7, 7, 0, 1, 1);

        TabGame_phase6 = new SequenceEdit("Player/phase6", groupBox_8);
        TabGame_phase6->setObjectName("TabGame_phase6");

        gridLayout_5->addWidget(TabGame_phase6, 6, 1, 1, 1);

        lbl_TabGame_phase8 = new QLabel(groupBox_8);
        lbl_TabGame_phase8->setObjectName("lbl_TabGame_phase8");

        gridLayout_5->addWidget(lbl_TabGame_phase8, 8, 0, 1, 1);

        TabGame_phase7 = new SequenceEdit("Player/phase7", groupBox_8);
        TabGame_phase7->setObjectName("TabGame_phase7");

        gridLayout_5->addWidget(TabGame_phase7, 7, 1, 1, 1);

        lbl_TabGame_phase9 = new QLabel(groupBox_8);
        lbl_TabGame_phase9->setObjectName("lbl_TabGame_phase9");

        gridLayout_5->addWidget(lbl_TabGame_phase9, 9, 0, 1, 1);

        TabGame_phase8 = new SequenceEdit("Player/phase8", groupBox_8);
        TabGame_phase8->setObjectName("TabGame_phase8");

        gridLayout_5->addWidget(TabGame_phase8, 8, 1, 1, 1);

        lbl_TabGame_phase10 = new QLabel(groupBox_8);
        lbl_TabGame_phase10->setObjectName("lbl_TabGame_phase10");

        gridLayout_5->addWidget(lbl_TabGame_phase10, 10, 0, 1, 1);

        TabGame_phase9 = new SequenceEdit("Player/phase9", groupBox_8);
        TabGame_phase9->setObjectName("TabGame_phase9");

        gridLayout_5->addWidget(TabGame_phase9, 9, 1, 1, 1);

        lbl_TabGame_aNextPhase = new QLabel(groupBox_8);
        lbl_TabGame_aNextPhase->setObjectName("lbl_TabGame_aNextPhase");

        gridLayout_5->addWidget(lbl_TabGame_aNextPhase, 11, 0, 1, 1);

        lbl_TabGame_aNextPhaseAction = new QLabel(groupBox_8);
        lbl_TabGame_aNextPhaseAction->setObjectName("lbl_TabGame_aNextPhaseAction");

        gridLayout_5->addWidget(lbl_TabGame_aNextPhaseAction, 12, 0, 1, 1);

        TabGame_phase10 = new SequenceEdit("Player/phase10", groupBox_8);
        TabGame_phase10->setObjectName("TabGame_phase10");

        gridLayout_5->addWidget(TabGame_phase10, 10, 1, 1, 1);

        lbl_TabGame_aNextTurn = new QLabel(groupBox_8);
        lbl_TabGame_aNextTurn->setObjectName("lbl_TabGame_aNextTurn");

        gridLayout_5->addWidget(lbl_TabGame_aNextTurn, 13, 0, 1, 1);

        TabGame_aNextPhase = new SequenceEdit("Player/aNextPhase", groupBox_8);
        TabGame_aNextPhase->setObjectName("TabGame_aNextPhase");

        gridLayout_5->addWidget(TabGame_aNextPhase, 11, 1, 1, 1);

        TabGame_aNextPhaseAction = new SequenceEdit("Player/aNextPhaseAction", groupBox_8);
        TabGame_aNextPhaseAction->setObjectName("TabGame_aaNextPhaseAction");

        gridLayout_5->addWidget(TabGame_aNextPhaseAction, 12, 1, 1, 1);

        TabGame_aNextTurn = new SequenceEdit("Player/aNextTurn", groupBox_8);
        TabGame_aNextTurn->setObjectName("TabGame_aNextTurn");

        gridLayout_5->addWidget(TabGame_aNextTurn, 13, 1, 1, 1);

        gridLayout_17->addWidget(groupBox_8, 0, 0, 1, 1);

        groupBox_13 = new QGroupBox(tab_2);
        groupBox_13->setObjectName("groupBox_13");
        gridLayout_13 = new QGridLayout(groupBox_13);
        gridLayout_13->setObjectName("gridLayout_13");
        lbl_Player_aTap = new QLabel(groupBox_13);
        lbl_Player_aTap->setObjectName("lbl_Player_aTap");

        gridLayout_13->addWidget(lbl_Player_aTap, 0, 0, 1, 1);

        Player_aTap = new SequenceEdit("Player/aTap", groupBox_13);
        Player_aTap->setObjectName("Player_aTap");

        gridLayout_13->addWidget(Player_aTap, 0, 1, 1, 1);

        lbl_Player_aUntapAll = new QLabel(groupBox_13);
        lbl_Player_aUntapAll->setObjectName("lbl_Player_aUntapAll");

        gridLayout_13->addWidget(lbl_Player_aUntapAll, 2, 0, 1, 1);

        Player_aUntapAll = new SequenceEdit("Player/aUntapAll", groupBox_13);
        Player_aUntapAll->setObjectName("Player_aUntapAll");

        gridLayout_13->addWidget(Player_aUntapAll, 2, 1, 1, 1);

        lbl_Player_aDoesntUntap = new QLabel(groupBox_13);
        lbl_Player_aDoesntUntap->setObjectName("lbl_Player_aDoesntUntap");

        gridLayout_13->addWidget(lbl_Player_aDoesntUntap, 3, 0, 1, 1);

        Player_aDoesntUntap = new SequenceEdit("Player/aDoesntUntap", groupBox_13);
        Player_aDoesntUntap->setObjectName("Player_aDoesntUntap");

        gridLayout_13->addWidget(Player_aDoesntUntap, 3, 1, 1, 1);

        lbl_Player_aFlip = new QLabel(groupBox_13);
        lbl_Player_aFlip->setObjectName("lbl_Player_aFlip");

        gridLayout_13->addWidget(lbl_Player_aFlip, 4, 0, 1, 1);

        Player_aFlip = new SequenceEdit("Player/aFlip", groupBox_13);
        Player_aFlip->setObjectName("Player_aFlip");

        gridLayout_13->addWidget(Player_aFlip, 4, 1, 1, 1);

        lbl_Player_aPeek = new QLabel(groupBox_13);
        lbl_Player_aPeek->setObjectName("lbl_Player_aPeek");

        gridLayout_13->addWidget(lbl_Player_aPeek, 5, 0, 1, 1);

        Player_aPeek = new SequenceEdit("Player/aPeek", groupBox_13);
        Player_aPeek->setObjectName("Player_aPeek");

        gridLayout_13->addWidget(Player_aPeek, 5, 1, 1, 1);

        lbl_Player_aPlay = new QLabel(groupBox_13);
        lbl_Player_aPlay->setObjectName("lbl_Player_aPlay");

        gridLayout_13->addWidget(lbl_Player_aPlay, 6, 0, 1, 1);

        Player_aPlay = new SequenceEdit("Player/aPlay", groupBox_13);
        Player_aPlay->setObjectName("Player_aPlay");

        gridLayout_13->addWidget(Player_aPlay, 6, 1, 1, 1);

        lbl_Player_aAttach = new QLabel(groupBox_13);
        lbl_Player_aAttach->setObjectName("lbl_Player_aAttach");

        gridLayout_13->addWidget(lbl_Player_aAttach, 7, 0, 1, 1);

        Player_aAttach = new SequenceEdit("Player/aAttach", groupBox_13);
        Player_aAttach->setObjectName("Player_aAttach");

        gridLayout_13->addWidget(Player_aAttach, 7, 1, 1, 1);

        lbl_Player_aUnattach = new QLabel(groupBox_13);
        lbl_Player_aUnattach->setObjectName("lbl_Player_aUnattach");

        gridLayout_13->addWidget(lbl_Player_aUnattach, 8, 0, 1, 1);

        Player_aUnattach = new SequenceEdit("Player/aUnattach", groupBox_13);
        Player_aUnattach->setObjectName("Player_aUnattach");

        gridLayout_13->addWidget(Player_aUnattach, 8, 1, 1, 1);

        lbl_Player_aClone = new QLabel(groupBox_13);
        lbl_Player_aClone->setObjectName("lbl_Player_aClone");

        gridLayout_13->addWidget(lbl_Player_aClone, 9, 0, 1, 1);

        Player_aClone = new SequenceEdit("Player/aClone", groupBox_13);
        Player_aClone->setObjectName("Player_aClone");

        gridLayout_13->addWidget(Player_aClone, 9, 1, 1, 1);

        lbl_Player_aCreateToken = new QLabel(groupBox_13);
        lbl_Player_aCreateToken->setObjectName("lbl_Player_aCreateToken");

        gridLayout_13->addWidget(lbl_Player_aCreateToken, 10, 0, 1, 1);

        Player_aCreateToken = new SequenceEdit("Player/aCreateToken", groupBox_13);
        Player_aCreateToken->setObjectName("Player_aCreateToken");

        gridLayout_13->addWidget(Player_aCreateToken, 10, 1, 1, 1);

        lbl_Player_aCreateRelatedTokens = new QLabel(groupBox_13);
        lbl_Player_aCreateRelatedTokens->setObjectName("lbl_Player_aCreateRelatedTokens");

        gridLayout_13->addWidget(lbl_Player_aCreateRelatedTokens, 11, 0, 1, 1);

        Player_aCreateRelatedTokens = new SequenceEdit("Player/aCreateRelatedTokens", groupBox_13);
        Player_aCreateRelatedTokens->setObjectName("Player_aCreateRelatedTokens");

        gridLayout_13->addWidget(Player_aCreateRelatedTokens, 11, 1, 1, 1);

        lbl_Player_aCreateAnotherToken = new QLabel(groupBox_13);
        lbl_Player_aCreateAnotherToken->setObjectName("lbl_Player_aCreateAnotherToken");

        gridLayout_13->addWidget(lbl_Player_aCreateAnotherToken, 12, 0, 1, 1);

        Player_aCreateAnotherToken = new SequenceEdit("Player/aCreateAnotherToken", groupBox_13);
        Player_aCreateAnotherToken->setObjectName("Player_aCreateAnotherToken");

        gridLayout_13->addWidget(Player_aCreateAnotherToken, 12, 1, 1, 1);

        lbl_Player_aSetAnnotation = new QLabel(groupBox_13);
        lbl_Player_aSetAnnotation->setObjectName("lbl_Player_aSetAnnotation");

        gridLayout_13->addWidget(lbl_Player_aSetAnnotation, 13, 0, 1, 1);

        Player_aSetAnnotation = new SequenceEdit("Player/aSetAnnotation", groupBox_13);
        Player_aSetAnnotation->setObjectName("Player_aSetAnnotation");

        gridLayout_13->addWidget(Player_aSetAnnotation, 13, 1, 1, 1);

        gridLayout_17->addWidget(groupBox_13, 0, 2, 1, 1);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_17->addItem(verticalSpacer_2, 1, 1, 1, 1);

        tabWidget->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName("tab_3");
        gridLayout_20 = new QGridLayout(tab_3);
        gridLayout_20->setObjectName("gridLayout_20");

        groupBox_gameplay = new QGroupBox(tab_3);
        groupBox_gameplay->setObjectName("groupBox_gameplay");
        gridLayout_gameplay = new QGridLayout(groupBox_gameplay);
        gridLayout_gameplay->setObjectName("gridLayout_gameplay");
        lbl_Player_aDrawArrow = new QLabel(groupBox_gameplay);
        lbl_Player_aDrawArrow->setObjectName("lbl_Player_aDrawArrow");

        gridLayout_gameplay->addWidget(lbl_Player_aDrawArrow, 0, 0, 1, 1);

        Player_aDrawArrow = new SequenceEdit("Player/aDrawArrow", groupBox_gameplay);
        Player_aDrawArrow->setObjectName("Player_aDrawArrow");

        gridLayout_gameplay->addWidget(Player_aDrawArrow, 0, 1, 1, 1);

        lbl_TabGame_aRemoveLocalArrows = new QLabel(groupBox_gameplay);
        lbl_TabGame_aRemoveLocalArrows->setObjectName("lbl_TabGame_aRemoveLocalArrows");

        gridLayout_gameplay->addWidget(lbl_TabGame_aRemoveLocalArrows, 1, 0, 1, 1);

        TabGame_aRemoveLocalArrows = new SequenceEdit("Player/aRemoveLocalArrows", groupBox_gameplay);
        TabGame_aRemoveLocalArrows->setObjectName("TabGame_aRemoveLocalArrows");

        gridLayout_gameplay->addWidget(TabGame_aRemoveLocalArrows, 1, 1, 1, 1);

        lbl_TabGame_aConcede = new QLabel(groupBox_gameplay);
        lbl_TabGame_aConcede->setObjectName("lbl_TabGame_aConcede");

        gridLayout_gameplay->addWidget(lbl_TabGame_aConcede, 2, 0, 1, 1);

        TabGame_aConcede = new SequenceEdit("Player/aConcede", groupBox_gameplay);
        TabGame_aConcede->setObjectName("TabGame_aConcede");

        gridLayout_gameplay->addWidget(TabGame_aConcede, 2, 1, 1, 1);

        lbl_TabGame_aLeaveGame = new QLabel(groupBox_gameplay);
        lbl_TabGame_aLeaveGame->setObjectName("lbl_TabGame_aLeaveGame");

        gridLayout_gameplay->addWidget(lbl_TabGame_aLeaveGame, 3, 0, 1, 1);

        TabGame_aLeaveGame = new SequenceEdit("Player/aLeaveGame", groupBox_gameplay);
        TabGame_aLeaveGame->setObjectName("TabGame_aLeaveGame");

        gridLayout_gameplay->addWidget(TabGame_aLeaveGame, 3, 1, 1, 1);

        lbl_Player_aRollDie = new QLabel(groupBox_gameplay);
        lbl_Player_aRollDie->setObjectName("lbl_Player_aRollDie");

        gridLayout_gameplay->addWidget(lbl_Player_aRollDie, 4, 0, 1, 1);

        Player_aRollDie = new SequenceEdit("Player/aRollDie", groupBox_gameplay);
        Player_aRollDie->setObjectName("Player_aRollDie");

        gridLayout_gameplay->addWidget(Player_aRollDie, 4, 1, 1, 1);

        lbl_Player_aShuffle = new QLabel(groupBox_gameplay);
        lbl_Player_aShuffle->setObjectName("lbl_Player_aShuffle");

        gridLayout_gameplay->addWidget(lbl_Player_aShuffle, 5, 0, 1, 1);

        Player_aShuffle = new SequenceEdit("Player/aShuffle", groupBox_gameplay);
        Player_aShuffle->setObjectName("Player_aShuffle");

        gridLayout_gameplay->addWidget(Player_aShuffle, 5, 1, 1, 1);

        lbl_TabGame_aRotateViewCW = new QLabel(groupBox_gameplay);
        lbl_TabGame_aRotateViewCW->setObjectName("lbl_TabGame_aRotateViewCW");

        gridLayout_gameplay->addWidget(lbl_TabGame_aRotateViewCW, 6, 0, 1, 1);

        TabGame_aRotateViewCW = new SequenceEdit("Player/aRotateViewCW", groupBox_gameplay);
        TabGame_aRotateViewCW->setObjectName("TabGame_aRotateViewCW");

        gridLayout_gameplay->addWidget(TabGame_aRotateViewCW, 6, 1, 1, 1);

        lbl_TabGame_aRotateViewCCW = new QLabel(groupBox_gameplay);
        lbl_TabGame_aRotateViewCCW->setObjectName("lbl_TabGame_aRotateViewCCW");

        gridLayout_gameplay->addWidget(lbl_TabGame_aRotateViewCCW, 7, 0, 1, 1);

        TabGame_aRotateViewCCW = new SequenceEdit("Player/aRotateViewCCW", groupBox_gameplay);
        TabGame_aRotateViewCCW->setObjectName("TabGame_aRotateViewCCW");

        gridLayout_gameplay->addWidget(TabGame_aRotateViewCCW, 7, 1, 1, 1);

        gridLayout_20->addWidget(groupBox_gameplay, 0, 0, 1, 1);

        groupBox_moveCard = new QGroupBox(tab_3);
        groupBox_moveCard->setObjectName("groupBox_moveCard");
        gridLayout_moveCard = new QGridLayout(groupBox_moveCard);
        gridLayout_moveCard->setObjectName("gridLayout_moveCard");
        lbl_Player_aMoveToBottomLibrary = new QLabel(groupBox_moveCard);
        lbl_Player_aMoveToBottomLibrary->setObjectName("lbl_Player_aMoveToBottomLibrary");

        gridLayout_moveCard->addWidget(lbl_Player_aMoveToBottomLibrary, 0, 0, 1, 1);

        Player_aMoveToBottomLibrary = new SequenceEdit("Player/aMoveToBottomLibrary", groupBox_moveCard);
        Player_aMoveToBottomLibrary->setObjectName("Player_aMoveToBottomLibrary");

        gridLayout_moveCard->addWidget(Player_aMoveToBottomLibrary, 0, 1, 1, 1);

        lbl_Player_aMoveToTopLibrary = new QLabel(groupBox_moveCard);
        lbl_Player_aMoveToTopLibrary->setObjectName("lbl_Player_aMoveToTopLibrary");

        gridLayout_moveCard->addWidget(lbl_Player_aMoveToTopLibrary, 1, 0, 1, 1);

        Player_aMoveToTopLibrary = new SequenceEdit("Player/aMoveToTopLibrary", groupBox_moveCard);
        Player_aMoveToTopLibrary->setObjectName("Player_aMoveToTopLibrary");

        gridLayout_moveCard->addWidget(Player_aMoveToTopLibrary, 1, 1, 1, 1);

        lbl_Player_aMoveToGraveyard = new QLabel(groupBox_moveCard);
        lbl_Player_aMoveToGraveyard->setObjectName("lbl_Player_aMoveToGraveyard");

        gridLayout_moveCard->addWidget(lbl_Player_aMoveToGraveyard, 2, 0, 1, 1);

        Player_aMoveToGraveyard = new SequenceEdit("Player/aMoveToGraveyard", groupBox_moveCard);
        Player_aMoveToGraveyard->setObjectName("Player_aMoveToGraveyard");

        gridLayout_moveCard->addWidget(Player_aMoveToGraveyard, 2, 1, 1, 1);

        lbl_Player_aMoveToExile = new QLabel(groupBox_moveCard);
        lbl_Player_aMoveToExile->setObjectName("lbl_Player_aMoveToExile");

        gridLayout_moveCard->addWidget(lbl_Player_aMoveToExile, 3, 0, 1, 1);

        Player_aMoveToExile = new SequenceEdit("Player/aMoveToExile", groupBox_moveCard);
        Player_aMoveToExile->setObjectName("Player_aMoveToExile");

        gridLayout_moveCard->addWidget(Player_aMoveToExile, 3, 1, 1, 1);

        lbl_Player_aMoveToHand = new QLabel(groupBox_moveCard);
        lbl_Player_aMoveToHand->setObjectName("lbl_Player_aMoveToHand");

        gridLayout_moveCard->addWidget(lbl_Player_aMoveToHand, 4, 0, 1, 1);

        Player_aMoveToHand = new SequenceEdit("Player/aMoveToHand", groupBox_moveCard);
        Player_aMoveToHand->setObjectName("Player_aMoveToHand");

        gridLayout_moveCard->addWidget(Player_aMoveToHand, 4, 1, 1, 1);

        lbl_Player_aMoveTopToPlayFaceDown = new QLabel(groupBox_moveCard);
        lbl_Player_aMoveTopToPlayFaceDown->setObjectName("lbl_Player_aMoveTopToPlayFaceDown");

        gridLayout_moveCard->addWidget(lbl_Player_aMoveTopToPlayFaceDown, 5, 0, 1, 1);

        Player_aMoveTopToPlayFaceDown = new SequenceEdit("Player/aMoveTopToPlayFaceDown", groupBox_moveCard);
        Player_aMoveTopToPlayFaceDown->setObjectName("Player_aMoveTopToPlayFaceDown");

        gridLayout_moveCard->addWidget(Player_aMoveTopToPlayFaceDown, 5, 1, 1, 1);

        gridLayout_20->addWidget(groupBox_moveCard, 0, 1, 1, 1);

        groupBox_view = new QGroupBox(tab_3);
        groupBox_view->setObjectName("groupBox_view");
        gridLayout_view = new QGridLayout(groupBox_view);
        gridLayout_view->setObjectName("gridLayout_view");
        lbl_Player_aViewGraveyard = new QLabel(groupBox_view);
        lbl_Player_aViewGraveyard->setObjectName("lbl_Player_aViewGraveyard");

        gridLayout_view->addWidget(lbl_Player_aViewGraveyard, 0, 0, 1, 1);

        Player_aViewGraveyard = new SequenceEdit("Player/aViewGraveyard", groupBox_view);
        Player_aViewGraveyard->setObjectName("Player_aViewGraveyard");

        gridLayout_view->addWidget(Player_aViewGraveyard, 0, 1, 1, 1);

        lbl_Player_aViewLibrary = new QLabel(groupBox_view);
        lbl_Player_aViewLibrary->setObjectName("lbl_Player_aViewLibrary");

        gridLayout_view->addWidget(lbl_Player_aViewLibrary, 1, 0, 1, 1);

        Player_aViewLibrary = new SequenceEdit("Player/aViewLibrary", groupBox_view);
        Player_aViewLibrary->setObjectName("Player_aViewLibrary");

        gridLayout_view->addWidget(Player_aViewLibrary, 1, 1, 1, 1);

        lbl_Player_aViewTopCards = new QLabel(groupBox_view);
        lbl_Player_aViewTopCards->setObjectName("lbl_Player_aViewTopCards");

        gridLayout_view->addWidget(lbl_Player_aViewTopCards, 2, 0, 1, 1);

        Player_aViewTopCards = new SequenceEdit("Player/aViewTopCards", groupBox_view);
        Player_aViewTopCards->setObjectName("Player_aViewTopCards");

        gridLayout_view->addWidget(Player_aViewTopCards, 2, 1, 1, 1);

        lbl_Player_aViewSideboard = new QLabel(groupBox_view);
        lbl_Player_aViewSideboard->setObjectName("lbl_Player_aViewSideboard");

        gridLayout_view->addWidget(lbl_Player_aViewSideboard, 3, 0, 1, 1);

        Player_aViewSideboard = new SequenceEdit("Player/aViewSideboard", groupBox_view);
        Player_aViewSideboard->setObjectName("Player_aViewSideboard");

        gridLayout_view->addWidget(Player_aViewSideboard, 3, 1, 1, 1);

        lbl_Player_aViewRfg = new QLabel(groupBox_view);
        lbl_Player_aViewRfg->setObjectName("lbl_Player_aViewRfg");

        gridLayout_view->addWidget(lbl_Player_aViewRfg, 4, 0, 1, 1);

        Player_aViewRfg = new SequenceEdit("Player/aViewRfg", groupBox_view);
        Player_aViewRfg->setObjectName("Player_aViewRfg");

        gridLayout_view->addWidget(Player_aViewRfg, 4, 1, 1, 1);

        lbl_GameView_aCloseMostRecentZoneView = new QLabel(groupBox_view);
        lbl_GameView_aCloseMostRecentZoneView->setObjectName("lbl_GameView_aCloseMostRecentZoneView");

        gridLayout_view->addWidget(lbl_GameView_aCloseMostRecentZoneView, 5, 0, 1, 1);

        GameView_aCloseMostRecentZoneView = new SequenceEdit("Player/aCloseMostRecentZoneView", groupBox_view);
        GameView_aCloseMostRecentZoneView->setObjectName("GameView_aCloseMostRecentZoneView");

        gridLayout_view->addWidget(GameView_aCloseMostRecentZoneView, 5, 1, 1, 1);

        gridLayout_20->addWidget(groupBox_view, 0, 2, 1, 1);

        groupBox_draw = new QGroupBox(tab_3);
        groupBox_draw->setObjectName("groupBox_draw");
        gridLayout_draw = new QGridLayout(groupBox_draw);
        gridLayout_draw->setObjectName("gridLayout_draw");
        lbl_Player_aMulligan = new QLabel(groupBox_draw);
        lbl_Player_aMulligan->setObjectName("lbl_Player_aMulligan");

        gridLayout_draw->addWidget(lbl_Player_aMulligan, 4, 0, 1, 1);

        Player_aMulligan = new SequenceEdit("Player/aMulligan", groupBox_draw);
        Player_aMulligan->setObjectName("Player_aMulligan");

        gridLayout_draw->addWidget(Player_aMulligan, 4, 1, 1, 1);

        lbl_Player_aDrawCard = new QLabel(groupBox_draw);
        lbl_Player_aDrawCard->setObjectName("lbl_Player_aDrawCard");

        gridLayout_draw->addWidget(lbl_Player_aDrawCard, 0, 0, 1, 1);

        Player_aDrawCard = new SequenceEdit("Player/aDrawCard", groupBox_draw);
        Player_aDrawCard->setObjectName("Player_aDrawCard");

        gridLayout_draw->addWidget(Player_aDrawCard, 0, 1, 1, 1);

        lbl_Player_aDrawCards = new QLabel(groupBox_draw);
        lbl_Player_aDrawCards->setObjectName("lbl_Player_aDrawCards");

        gridLayout_draw->addWidget(lbl_Player_aDrawCards, 1, 0, 1, 1);

        Player_aDrawCards = new SequenceEdit("Player/aDrawCards", groupBox_draw);
        Player_aDrawCards->setObjectName("Player_aDrawCards");

        gridLayout_draw->addWidget(Player_aDrawCards, 1, 1, 1, 1);

        lbl_Player_aUndoDraw = new QLabel(groupBox_draw);
        lbl_Player_aUndoDraw->setObjectName("lbl_Player_aUndoDraw");

        gridLayout_draw->addWidget(lbl_Player_aUndoDraw, 2, 0, 1, 1);

        Player_aUndoDraw = new SequenceEdit("Player/aUndoDraw", groupBox_draw);
        Player_aUndoDraw->setObjectName("Player_aUndoDraw");

        gridLayout_draw->addWidget(Player_aUndoDraw, 2, 1, 1, 1);

        lbl_Player_aAlwaysRevealTopCard = new QLabel(groupBox_draw);
        lbl_Player_aAlwaysRevealTopCard->setObjectName("lbl_Player_aAlwaysRevealTopCard");

        gridLayout_draw->addWidget(lbl_Player_aAlwaysRevealTopCard, 3, 0, 1, 1);

        Player_aAlwaysRevealTopCard = new SequenceEdit("Player/aAlwaysRevealTopCard", groupBox_draw);
        Player_aAlwaysRevealTopCard->setObjectName("Player_aAlwaysRevealTopCard");

        gridLayout_draw->addWidget(Player_aAlwaysRevealTopCard, 3, 1, 1, 1);
        gridLayout_20->addWidget(groupBox_draw, 1, 0, 1, 1);

        groupBox_moveDeck = new QGroupBox(tab_3);
        groupBox_moveDeck->setObjectName("groupBox_moveDeck");
        gridLayout_moveDeck = new QGridLayout(groupBox_moveDeck);
        gridLayout_moveDeck->setObjectName("gridLayout_moveDeck");
        lbl_Player_aMoveTopCardToGraveyard = new QLabel(groupBox_moveDeck);
        lbl_Player_aMoveTopCardToGraveyard->setObjectName("lbl_Player_aMoveTopCardToGraveyard");

        gridLayout_moveDeck->addWidget(lbl_Player_aMoveTopCardToGraveyard, 0, 0, 1, 1);

        Player_aMoveTopCardToGraveyard = new SequenceEdit("Player/aMoveTopCardToGraveyard", groupBox_moveDeck);
        Player_aMoveTopCardToGraveyard->setObjectName("Player_aMoveTopCardToGraveyard");

        gridLayout_moveDeck->addWidget(Player_aMoveTopCardToGraveyard, 0, 1, 1, 1);

        lbl_Player_aMoveTopCardsToGraveyard = new QLabel(groupBox_moveDeck);
        lbl_Player_aMoveTopCardsToGraveyard->setObjectName("lbl_Player_aMoveTopCardsToGraveyard");

        gridLayout_moveDeck->addWidget(lbl_Player_aMoveTopCardsToGraveyard, 1, 0, 1, 1);

        Player_aMoveTopCardsToGraveyard = new SequenceEdit("Player/aMoveTopCardsToGraveyard", groupBox_moveDeck);
        Player_aMoveTopCardsToGraveyard->setObjectName("Player_aMoveTopCardsToGraveyard");

        gridLayout_moveDeck->addWidget(Player_aMoveTopCardsToGraveyard, 1, 1, 1, 1);

        lbl_Player_aMoveTopCardToExile = new QLabel(groupBox_moveDeck);
        lbl_Player_aMoveTopCardToExile->setObjectName("lbl_Player_aMoveTopCardToExile");

        gridLayout_moveDeck->addWidget(lbl_Player_aMoveTopCardToExile, 2, 0, 1, 1);

        Player_aMoveTopCardToExile = new SequenceEdit("Player/aMoveTopCardToExile", groupBox_moveDeck);
        Player_aMoveTopCardToExile->setObjectName("Player_aMoveTopCardToExile");

        gridLayout_moveDeck->addWidget(Player_aMoveTopCardToExile, 2, 1, 1, 1);

        lbl_Player_aMoveTopCardsToExile = new QLabel(groupBox_moveDeck);
        lbl_Player_aMoveTopCardsToExile->setObjectName("lbl_Player_aMoveTopCardsToExile");

        gridLayout_moveDeck->addWidget(lbl_Player_aMoveTopCardsToExile, 3, 0, 1, 1);

        Player_aMoveTopCardsToExile = new SequenceEdit("Player/aMoveTopCardsToExile", groupBox_moveDeck);
        Player_aMoveTopCardsToExile->setObjectName("Player_aMoveTopCardsToExile");

        gridLayout_moveDeck->addWidget(Player_aMoveTopCardsToExile, 3, 1, 1, 1);

        gridLayout_20->addWidget(groupBox_moveDeck, 1, 1, 1, 1);

        groupBox_gameLobby = new QGroupBox(tab_3);
        groupBox_gameLobby->setObjectName("groupBox_gameLobby");
        gridLayout_gameLobby = new QGridLayout(groupBox_gameLobby);
        gridLayout_gameLobby->setObjectName("gridLayout_gameLobby");
        DeckViewContainer_loadRemoteButton = new SequenceEdit("DeckViewContainer/loadRemoteButton", groupBox_gameLobby);
        DeckViewContainer_loadRemoteButton->setObjectName("DeckViewContainer_loadRemoteButton");

        gridLayout_gameLobby->addWidget(DeckViewContainer_loadRemoteButton, 2, 1, 1, 1);

        DeckViewContainer_loadLocalButton = new SequenceEdit("DeckViewContainer/loadLocalButton", groupBox_gameLobby);
        DeckViewContainer_loadLocalButton->setObjectName("DeckViewContainer_loadLocalButton");

        gridLayout_gameLobby->addWidget(DeckViewContainer_loadLocalButton, 0, 1, 1, 1);

        lbl_DeckViewContainer_loadRemoteButton = new QLabel(groupBox_gameLobby);
        lbl_DeckViewContainer_loadRemoteButton->setObjectName("lbl_DeckViewContainer_loadRemoteButton");

        gridLayout_gameLobby->addWidget(lbl_DeckViewContainer_loadRemoteButton, 2, 0, 1, 1);

        lbl_DeckViewContainer_loadLocalButton = new QLabel(groupBox_gameLobby);
        lbl_DeckViewContainer_loadLocalButton->setObjectName("lbl_DeckViewContainer_loadLocalButton");

        gridLayout_gameLobby->addWidget(lbl_DeckViewContainer_loadLocalButton, 0, 0, 1, 1);

        gridLayout_20->addWidget(groupBox_gameLobby, 1, 2, 1, 1);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        gridLayout_20->addItem(verticalSpacer_3, 2, 1, 1, 1);
        tabWidget->addTab(tab_3, QString());
        tab_4 = new QWidget(tabWidget);
        auto *grid = new QGridLayout(tab_4);
        grid->addWidget(groupBox_3);
        grid->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding), 1, 0);

        tabWidget->addTab(tab_4, QString());

        faqLabel = new QLabel(shortcutsTab);
        faqLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        faqLabel->setOpenExternalLinks(true);

        btnResetAll = new QPushButton(shortcutsTab);
        btnClearAll = new QPushButton(shortcutsTab);

        btnResetAll->setIcon(QPixmap("theme:icons/update"));
        btnClearAll->setIcon(QPixmap("theme:icons/clearsearch"));

        auto *buttonsLayout = new QHBoxLayout(shortcutsTab);
        buttonsLayout->addWidget(btnClearAll);
        buttonsLayout->addWidget(btnResetAll);

        gridLayout_9->addWidget(tabWidget, 0, 0, 1, 2);
        gridLayout_9->addWidget(faqLabel, 1, 0, 1, 1);
        gridLayout_9->addLayout(buttonsLayout, 1, 1, 1, 1, Qt::AlignRight);
        tabWidget->setCurrentIndex(0);

        grid->setSpacing(3);
        gridLayout->setSpacing(3);
        gridLayout_2->setSpacing(3);
        gridLayout_3->setSpacing(3);
        gridLayout_4->setSpacing(3);
        gridLayout_5->setSpacing(3);
        gridLayout_6->setSpacing(3);
        gridLayout_7->setSpacing(3);
        gridLayout_8->setSpacing(3);
        gridLayout_9->setSpacing(3);
        gridLayout_10->setSpacing(3);
        gridLayout_11->setSpacing(3);
        gridLayout_12->setSpacing(3);
        gridLayout_13->setSpacing(3);
        gridLayout_moveDeck->setSpacing(3);
        gridLayout_draw->setSpacing(3);
        gridLayout_moveCard->setSpacing(3);
        gridLayout_view->setSpacing(3);
        gridLayout_gameLobby->setSpacing(3);
        gridLayout_gameplay->setSpacing(3);
        gridLayout_17->setSpacing(3);
        gridLayout_20->setSpacing(3);

        verticalLayout->setSpacing(3);
        counterLayout->setSpacing(3);
        QMetaObject::connectSlotsByName(shortcutsTab);
        retranslateUi(shortcutsTab);
    } // setupUi

    void retranslateUi(QWidget * /*shortcutsTab */)
    {
        groupBox->setTitle(QApplication::translate("shortcutsTab", "Main Window"));
        lbl_MainWindow_aDeckEditor->setText(QApplication::translate("shortcutsTab", "Deck editor"));
        lbl_MainWindow_aSinglePlayer->setText(QApplication::translate("shortcutsTab", "Local gameplay"));
        lbl_MainWindow_aWatchReplay->setText(QApplication::translate("shortcutsTab", "Watch replay"));
        lbl_MainWindow_aConnect->setText(QApplication::translate("shortcutsTab", "Connect"));
        lbl_MainWindow_aRegister->setText(QApplication::translate("shortcutsTab", "Register"));
        lbl_MainWindow_aFullScreen->setText(QApplication::translate("shortcutsTab", "Full screen"));
        lbl_MainWindow_aSettings->setText(QApplication::translate("shortcutsTab", "Settings"));
        lbl_MainWindow_aCheckCardUpdates->setText(QApplication::translate("shortcutsTab", "Check for card updates"));
        lbl_MainWindow_aDisconnect->setText(QApplication::translate("shortcutsTab", "Disconnect"));
        lbl_MainWindow_aExit->setText(QApplication::translate("shortcutsTab", "Exit"));
        groupBox_2->setTitle(QApplication::translate("shortcutsTab", "Deck Editor"));
        lbl_TabDeckEditor_aAnalyzeDeck->setText(QApplication::translate("shortcutsTab", "Analyze deck"));
        lbl_TabDeckEditor_aLoadDeckFromClipboard->setText(
            QApplication::translate("shortcutsTab", "Load deck (clipboard)"));
        lbl_TabDeckEditor_aClearFilterAll->setText(QApplication::translate("shortcutsTab", "Clear all filters"));
        lbl_TabDeckEditor_aNewDeck->setText(QApplication::translate("shortcutsTab", "New deck"));
        lbl_TabDeckEditor_aClearFilterOne->setText(QApplication::translate("shortcutsTab", "Clear selected filter"));
        lbl_TabDeckEditor_aOpenCustomFolder->setText(QApplication::translate("shortcutsTab", "Open custom pic folder"));
        lbl_TabDeckEditor_aClose->setText(QApplication::translate("shortcutsTab", "Close"));
        lbl_TabDeckEditor_aPrintDeck->setText(QApplication::translate("shortcutsTab", "Print deck"));
        lbl_TabDeckEditor_aManageSets->setText(QApplication::translate("shortcutsTab", "Manage sets"));
        lbl_TabDeckEditor_aRemoveCard->setText(QApplication::translate("shortcutsTab", "Delete card"));
        lbl_TabDeckEditor_aEditTokens->setText(QApplication::translate("shortcutsTab", "Edit tokens"));
        lbl_TabDeckEditor_aResetLayout->setText(QApplication::translate("shortcutsTab", "Reset layout"));
        lbl_TabDeckEditor_aIncrement->setText(QApplication::translate("shortcutsTab", "Add card"));
        lbl_TabDeckEditor_aSaveDeck->setText(QApplication::translate("shortcutsTab", "Save deck"));
        lbl_TabDeckEditor_aExportDeckDecklist->setText(QApplication::translate("shortcutsTab", "Export deck"));
        lbl_TabDeckEditor_aDecrement->setText(QApplication::translate("shortcutsTab", "Remove card"));
        lbl_TabDeckEditor_aSaveDeckAs->setText(QApplication::translate("shortcutsTab", "Save deck as"));
        lbl_TabDeckEditor_aLoadDeck->setText(QApplication::translate("shortcutsTab", "Load deck"));
        lbl_TabDeckEditor_aSaveDeckToClipboard->setText(QApplication::translate("shortcutsTab", "Save deck (clip)"));
        lbl_TabDeckEditor_aSaveDeckToClipboardRaw->setText(
            QApplication::translate("shortcutsTab", "Save deck (clip; no annotations)"));
        groupBox_3->setTitle(QApplication::translate("shortcutsTab", "Counters"));
        groupBox_4->setTitle(QApplication::translate("shortcutsTab", "Life"));
        lbl_abstractCounter_sSet->setText(QApplication::translate("shortcutsTab", "Set"));
        lbl_abstractCounter_aInc->setText(QApplication::translate("shortcutsTab", "Add"));
        lbl_abstractCounter_aDec->setText(QApplication::translate("shortcutsTab", "Remove"));
        groupBox_5->setTitle(QApplication::translate("shortcutsTab", "Red"));
        lbl_Player_aSCRed->setText(QApplication::translate("shortcutsTab", "Set"));
        lbl_Player_aCCRed->setText(QApplication::translate("shortcutsTab", "Add"));
        lbl_Player_aRCRed->setText(QApplication::translate("shortcutsTab", "Remove"));
        groupBox_6->setTitle(QApplication::translate("shortcutsTab", "Green"));
        lbl_Player_aSCGreen->setText(QApplication::translate("shortcutsTab", "Set"));
        lbl_Player_aCCGreen->setText(QApplication::translate("shortcutsTab", "Add"));
        lbl_Player_aRCGreen->setText(QApplication::translate("shortcutsTab", "Remove"));
        groupBox_7->setTitle(QApplication::translate("shortcutsTab", "Yellow"));
        lbl_Player_aSCYellow->setText(QApplication::translate("shortcutsTab", "Set"));
        lbl_Player_aCCYellow->setText(QApplication::translate("shortcutsTab", "Add"));
        lbl_Player_aRCYellow->setText(QApplication::translate("shortcutsTab", "Remove"));

        groupBox_counterStorm->setTitle(QApplication::translate("shortcutsTab", "Storm"));
        lbl_Player_aSetCStorm->setText(QApplication::translate("shortcutsTab", "Set"));
        lbl_Player_aIncCStorm->setText(QApplication::translate("shortcutsTab", "Add"));
        lbl_Player_aDecCStorm->setText(QApplication::translate("shortcutsTab", "Remove"));
        groupBox_counterW->setTitle(QApplication::translate("shortcutsTab", "W"));
        lbl_Player_aSetCW->setText(QApplication::translate("shortcutsTab", "Set"));
        lbl_Player_aIncCW->setText(QApplication::translate("shortcutsTab", "Add"));
        lbl_Player_aDecCW->setText(QApplication::translate("shortcutsTab", "Remove"));
        groupBox_counterU->setTitle(QApplication::translate("shortcutsTab", "U"));
        lbl_Player_aSetCU->setText(QApplication::translate("shortcutsTab", "Set"));
        lbl_Player_aIncCU->setText(QApplication::translate("shortcutsTab", "Add"));
        lbl_Player_aDecCU->setText(QApplication::translate("shortcutsTab", "Remove"));
        groupBox_counterB->setTitle(QApplication::translate("shortcutsTab", "B"));
        lbl_Player_aSetCB->setText(QApplication::translate("shortcutsTab", "Set"));
        lbl_Player_aIncCB->setText(QApplication::translate("shortcutsTab", "Add"));
        lbl_Player_aDecCB->setText(QApplication::translate("shortcutsTab", "Remove"));
        groupBox_counterR->setTitle(QApplication::translate("shortcutsTab", "R"));
        lbl_Player_aSetCR->setText(QApplication::translate("shortcutsTab", "Set"));
        lbl_Player_aIncCR->setText(QApplication::translate("shortcutsTab", "Add"));
        lbl_Player_aDecCR->setText(QApplication::translate("shortcutsTab", "Remove"));
        groupBox_counterG->setTitle(QApplication::translate("shortcutsTab", "G"));
        lbl_Player_aSetCG->setText(QApplication::translate("shortcutsTab", "Set"));
        lbl_Player_aIncCG->setText(QApplication::translate("shortcutsTab", "Add"));
        lbl_Player_aDecCG->setText(QApplication::translate("shortcutsTab", "Remove"));
        groupBox_counterX->setTitle(QApplication::translate("shortcutsTab", "X"));
        lbl_Player_aSetCX->setText(QApplication::translate("shortcutsTab", "Set"));
        lbl_Player_aIncCX->setText(QApplication::translate("shortcutsTab", "Add"));
        lbl_Player_aDecCX->setText(QApplication::translate("shortcutsTab", "Remove"));

        lbl_Player_aSCYellow->setText(QApplication::translate("shortcutsTab", "Set"));
        lbl_Player_aCCYellow->setText(QApplication::translate("shortcutsTab", "Add"));
        lbl_Player_aRCYellow->setText(QApplication::translate("shortcutsTab", "Remove"));
        tabWidget->setTabText(tabWidget->indexOf(tab),
                              QApplication::translate("shortcutsTab", "Main Window | Deck Editor"));
        groupBox_9->setTitle(QApplication::translate("shortcutsTab", "Power / Toughness"));
        groupBox_12->setTitle(QApplication::translate("shortcutsTab", "Power and Toughness"));
        lbl_Player_aIncPT->setText(QApplication::translate("shortcutsTab", "Add (+1/+1)"));
        lbl_Player_aDecPT->setText(QApplication::translate("shortcutsTab", "Remove (-1/-1)"));
        lbl_Player_aResetPT->setText(QApplication::translate("shortcutsTab", "Reset"));
        lbl_Player_aSetPT->setText(QApplication::translate("shortcutsTab", "Set"));
        groupBox_11->setTitle(QApplication::translate("shortcutsTab", "Toughness"));
        lbl_Player_aDecT->setText(QApplication::translate("shortcutsTab", "Remove (-0/-1)"));
        lbl_Player_aIncT->setText(QApplication::translate("shortcutsTab", "Add (+0/+1)"));
        groupBox_10->setTitle(QApplication::translate("shortcutsTab", "Power"));
        lbl_Player_aDecP->setText(QApplication::translate("shortcutsTab", "Remove (-1/-nullptr)"));
        lbl_Player_aIncP->setText(QApplication::translate("shortcutsTab", "Add (+1/+nullptr)"));
        groupBox_8->setTitle(QApplication::translate("shortcutsTab", "Game Phases"));
        lbl_TabGame_phase0->setText(QApplication::translate("shortcutsTab", "Untap"));
        lbl_TabGame_phase1->setText(QApplication::translate("shortcutsTab", "Upkeep"));
        lbl_TabGame_phase2->setText(QApplication::translate("shortcutsTab", "Draw"));
        lbl_TabGame_phase3->setText(QApplication::translate("shortcutsTab", "Main 1"));
        lbl_TabGame_phase4->setText(QApplication::translate("shortcutsTab", "Start combat"));
        lbl_TabGame_phase5->setText(QApplication::translate("shortcutsTab", "Attack"));
        lbl_TabGame_phase6->setText(QApplication::translate("shortcutsTab", "Block"));
        lbl_TabGame_phase7->setText(QApplication::translate("shortcutsTab", "Damage"));
        lbl_TabGame_phase8->setText(QApplication::translate("shortcutsTab", "End combat"));
        lbl_TabGame_phase9->setText(QApplication::translate("shortcutsTab", "Main 2"));
        lbl_TabGame_phase10->setText(QApplication::translate("shortcutsTab", "End"));
        lbl_TabGame_aNextPhase->setText(QApplication::translate("shortcutsTab", "Next phase"));
        lbl_TabGame_aNextPhaseAction->setText(QApplication::translate("shortcutsTab", "Next phase action"));
        lbl_TabGame_aNextTurn->setText(QApplication::translate("shortcutsTab", "Next turn"));
        groupBox_13->setTitle(QApplication::translate("shortcutsTab", "Playing Area"));
        lbl_Player_aTap->setText(QApplication::translate("shortcutsTab", "Tap / Untap Card"));
        lbl_Player_aUntapAll->setText(QApplication::translate("shortcutsTab", "Untap all"));
        lbl_Player_aDoesntUntap->setText(QApplication::translate("shortcutsTab", "Toggle untap"));
        lbl_Player_aFlip->setText(QApplication::translate("shortcutsTab", "Flip card"));
        lbl_Player_aPeek->setText(QApplication::translate("shortcutsTab", "Peek card"));
        lbl_Player_aPlay->setText(QApplication::translate("shortcutsTab", "Play card"));
        lbl_Player_aAttach->setText(QApplication::translate("shortcutsTab", "Attach card"));
        lbl_Player_aUnattach->setText(QApplication::translate("shortcutsTab", "Unattach card"));
        lbl_Player_aClone->setText(QApplication::translate("shortcutsTab", "Clone card"));
        lbl_Player_aCreateToken->setText(QApplication::translate("shortcutsTab", "Create token"));
        lbl_Player_aCreateRelatedTokens->setText(QApplication::translate("shortcutsTab", "Create all related tokens"));
        lbl_Player_aCreateAnotherToken->setText(QApplication::translate("shortcutsTab", "Create another token"));
        lbl_Player_aSetAnnotation->setText(QApplication::translate("shortcutsTab", "Set annotation"));
        tabWidget->setTabText(tabWidget->indexOf(tab_2),
                              QApplication::translate("shortcutsTab", "Phases | P/T | Playing Area"));
        groupBox_moveCard->setTitle(QApplication::translate("shortcutsTab", "Move selected card to"));
        lbl_Player_aMoveToBottomLibrary->setText(QApplication::translate("shortcutsTab", "Bottom library"));
        lbl_Player_aMoveToTopLibrary->setText(QApplication::translate("shortcutsTab", "Top library"));
        lbl_Player_aMoveToGraveyard->setText(QApplication::translate("shortcutsTab", "Graveyard"));
        lbl_Player_aMoveToExile->setText(QApplication::translate("shortcutsTab", "Exile"));
        lbl_Player_aMoveToHand->setText(QApplication::translate("shortcutsTab", "Hand"));
        lbl_Player_aMoveTopToPlayFaceDown->setText(QApplication::translate("shortcutsTab", "Play face down"));
        groupBox_view->setTitle(QApplication::translate("shortcutsTab", "View"));
        lbl_Player_aViewGraveyard->setText(QApplication::translate("shortcutsTab", "Graveyard"));
        lbl_Player_aViewLibrary->setText(QApplication::translate("shortcutsTab", "Library"));
        lbl_Player_aViewTopCards->setText(QApplication::translate("shortcutsTab", "Top cards of library"));
        lbl_Player_aViewSideboard->setText(QApplication::translate("shortcutsTab", "Sideboard"));
        lbl_Player_aViewRfg->setText(QApplication::translate("shortcutsTab", "Exile"));
        lbl_GameView_aCloseMostRecentZoneView->setText(QApplication::translate("shortcutsTab", "Close recent view"));
        groupBox_moveDeck->setTitle(QApplication::translate("shortcutsTab", "Move top card to"));
        lbl_Player_aMoveTopCardToGraveyard->setText(QApplication::translate("shortcutsTab", "Graveyard Once"));
        lbl_Player_aMoveTopCardsToGraveyard->setText(QApplication::translate("shortcutsTab", "Graveyard Multiple"));
        lbl_Player_aMoveTopCardToExile->setText(QApplication::translate("shortcutsTab", "Exile Once"));
        lbl_Player_aMoveTopCardsToExile->setText(QApplication::translate("shortcutsTab", "Exile Multiple"));
        groupBox_gameLobby->setTitle(QApplication::translate("shortcutsTab", "Game Lobby"));
        lbl_DeckViewContainer_loadRemoteButton->setText(QApplication::translate("shortcutsTab", "Load remote deck"));
        lbl_DeckViewContainer_loadLocalButton->setText(QApplication::translate("shortcutsTab", "Load local deck"));
        groupBox_gameplay->setTitle(QApplication::translate("shortcutsTab", "Gameplay"));
        lbl_Player_aDrawArrow->setText(QApplication::translate("shortcutsTab", "Draw arrow"));
        lbl_TabGame_aLeaveGame->setText(QApplication::translate("shortcutsTab", "Leave game"));
        lbl_TabGame_aRemoveLocalArrows->setText(QApplication::translate("shortcutsTab", "Remove local arrows"));
        lbl_TabGame_aConcede->setText(QApplication::translate("shortcutsTab", "Concede"));
        lbl_Player_aRollDie->setText(QApplication::translate("shortcutsTab", "Roll dice"));
        lbl_TabGame_aRotateViewCW->setText(QApplication::translate("shortcutsTab", "Rotate view CW"));
        lbl_Player_aShuffle->setText(QApplication::translate("shortcutsTab", "Shuffle library"));
        lbl_TabGame_aRotateViewCCW->setText(QApplication::translate("shortcutsTab", "Rotate view CCW"));
        groupBox_draw->setTitle(QApplication::translate("shortcutsTab", "Drawing"));
        lbl_Player_aMulligan->setText(QApplication::translate("shortcutsTab", "Mulligan"));
        lbl_Player_aDrawCard->setText(QApplication::translate("shortcutsTab", "Draw card"));
        lbl_Player_aDrawCards->setText(QApplication::translate("shortcutsTab", "Draw cards"));
        lbl_Player_aUndoDraw->setText(QApplication::translate("shortcutsTab", "Undo draw"));
        lbl_Player_aAlwaysRevealTopCard->setText(QApplication::translate("shortcutsTab", "Always reveal top card"));
        tabWidget->setTabText(tabWidget->indexOf(tab_3),
                              QApplication::translate("shortcutsTab", "Gameplay | Draw | Move | View"));
        tabWidget->setTabText(tabWidget->indexOf(tab_4), QApplication::translate("shortcutsTab", "Counters"));
        faqLabel->setText(QString("<a href='%1'>%2</a>")
                              .arg(WIKI)
                              .arg(QApplication::translate("shortcutsTab", "How to set custom shortcuts")));
        btnResetAll->setText(QApplication::translate("shortcutsTab", "Restore all default shortcuts"));
        btnClearAll->setText(QApplication::translate("shortcutsTab", "Clear all shortcuts"));
    } // retranslateUi
};

namespace Ui
{
class shortcutsTab : public Ui_shortcutsTab
{
};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SHORTCUTSTAB_H
