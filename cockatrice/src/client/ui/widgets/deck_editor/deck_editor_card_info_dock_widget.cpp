#include "deck_editor_card_info_dock_widget.h"

#include "../cards/card_info_frame_widget.h"

#include <QVBoxLayout>

DeckEditorCardInfoDockWidget::DeckEditorCardInfoDockWidget(QWidget *parent, TabGenericDeckEditor *_deckEditor)
    : QDockWidget(parent), deckEditor(_deckEditor)
{
    setObjectName("cardInfoDock");

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);

    createCardInfoDock();

    retranslateUi();
}

void DeckEditorCardInfoDockWidget::createCardInfoDock()
{
    cardInfo = new CardInfoFrameWidget();
    cardInfo->setObjectName("cardInfo");
    auto *cardInfoFrame = new QVBoxLayout;
    cardInfoFrame->setObjectName("cardInfoFrame");
    cardInfoFrame->addWidget(cardInfo);

    auto *cardInfoDockContents = new QWidget();
    cardInfoDockContents->setObjectName("cardInfoDockContents");
    cardInfoDockContents->setLayout(cardInfoFrame);
    setWidget(cardInfoDockContents);

    installEventFilter(deckEditor);
    connect(this, &QDockWidget::topLevelChanged, deckEditor, &TabGenericDeckEditor::dockTopLevelChanged);
}

void DeckEditorCardInfoDockWidget::retranslateUi()
{
    setWindowTitle(tr("Card Info"));
    cardInfo->retranslateUi();
}