#include "deck_editor_card_info_dock_widget.h"

#include "../cards/card_info_frame_widget.h"

#include <QVBoxLayout>

DeckEditorCardInfoDockWidget::DeckEditorCardInfoDockWidget(QWidget *parent) : QDockWidget(parent)
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

    installEventFilter(this);
    // connect(this, SIGNAL(topLevelChanged(bool)), deckEditor, SLOT(dockTopLevelChanged(bool)));
}

void DeckEditorCardInfoDockWidget::retranslateUi()
{
    setWindowTitle(tr("Card Info"));
    cardInfo->retranslateUi();
}