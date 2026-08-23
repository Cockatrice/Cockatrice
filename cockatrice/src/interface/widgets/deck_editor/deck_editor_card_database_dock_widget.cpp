#include "deck_editor_card_database_dock_widget.h"

#include "../../../interface/widgets/tabs/abstract_tab_deck_editor.h"
#include "card_database_view.h"
#include "deck_state_manager.h"
#include "deck_zone_dialog.h"

#include <libcockatrice/deck_list/deck_list_node_tree.h>

DeckEditorCardDatabaseDockWidget::DeckEditorCardDatabaseDockWidget(AbstractTabDeckEditor *parent) : QDockWidget(parent)
{
    setObjectName("databaseDisplayDock");
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);

    createDatabaseDisplayDock(parent);

    retranslateUi();
}

void DeckEditorCardDatabaseDockWidget::createDatabaseDisplayDock(AbstractTabDeckEditor *deckEditor)
{
    databaseDisplayWidget = new DeckEditorDatabaseDisplayWidget(this, deckEditor->databaseModel);

    databaseDisplayWidget->getDatabaseView()->setZoneMenuProvider(
        [deckEditor]() -> QList<QPair<QString, QStringList>> {
            QList<QPair<QString, QStringList>> result;
            auto *deckListModel = deckEditor->deckStateManager->getModel();
            for (const QString &boardName : InnerDecklistNode::boardZoneNames()) {
                result.append({boardName, deckListModel->getCustomZoneNames(boardName)});
            }
            return result;
        },
        [this, deckEditor] {
            QString boardName;
            const QString zoneName =
                DeckZoneDialog::promptForNewZone(this, {}, &boardName, [deckEditor](const QString &candidate) {
                    return deckEditor->deckStateManager->validateNewZoneName(candidate);
                });
            if (!zoneName.isEmpty()) {
                deckEditor->deckStateManager->createCustomZone(boardName, zoneName);
            }
        });

    auto *frame = new QVBoxLayout;
    frame->setObjectName("databaseDisplayFrame");
    frame->addWidget(databaseDisplayWidget);

    auto *dockContents = new QWidget();
    dockContents->setObjectName("databaseDisplayDockContents");
    dockContents->setLayout(frame);
    setWidget(dockContents);

    installEventFilter(deckEditor);

    // connect signals
    connect(databaseDisplayWidget, &DeckEditorDatabaseDisplayWidget::cardChanged, deckEditor,
            &AbstractTabDeckEditor::updateCard);
    connect(databaseDisplayWidget, &DeckEditorDatabaseDisplayWidget::cardAdded, deckEditor,
            &AbstractTabDeckEditor::addCard);
    connect(databaseDisplayWidget, &DeckEditorDatabaseDisplayWidget::cardDecremented, deckEditor,
            &AbstractTabDeckEditor::decrementCard);
    connect(databaseDisplayWidget, &DeckEditorDatabaseDisplayWidget::edhrecRequested, deckEditor,
            &AbstractTabDeckEditor::openEdhrecTab);
    connect(databaseDisplayWidget, &DeckEditorDatabaseDisplayWidget::printingSelectorRequested, deckEditor,
            &AbstractTabDeckEditor::showPrintingSelector);
    connect(databaseDisplayWidget, &DeckEditorDatabaseDisplayWidget::cardInfoRequested, deckEditor,
            &AbstractTabDeckEditor::updateCardInfo);
}

void DeckEditorCardDatabaseDockWidget::retranslateUi()
{
    setWindowTitle(tr("Card Database"));
}

void DeckEditorCardDatabaseDockWidget::setFilterTree(FilterTree *filterTree)
{
    databaseDisplayWidget->setFilterTree(filterTree);
}

void DeckEditorCardDatabaseDockWidget::clearAllDatabaseFilters()
{
    databaseDisplayWidget->clearAllDatabaseFilters();
}
