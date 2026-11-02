#include "tab_visual_database_display.h"

#include "../../../client/settings/cache_settings.h"
#include "../cards/card_info_frame_widget.h"
#include "../deck_editor/card_database_view.h"
#include "../visual_database_display/visual_database_display_widget.h"
#include "card_database_display_model.h"
#include "card_database_model.h"
#include "tab_supervisor.h"

#include <QAction>
#include <QDockWidget>
#include <QHideEvent>
#include <QItemSelectionModel>
#include <QLabel>
#include <QMenu>
#include <QVBoxLayout>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/settings/layouts_settings.h>

TabVisualDatabaseDisplay::TabVisualDatabaseDisplay(TabSupervisor *_tabSupervisor) : Tab(_tabSupervisor)
{
    auto databaseModel = new CardDatabaseModel(CardDatabaseManager::getInstance(), true, this);
    databaseModel->setObjectName("databaseModel");

    visualDatabaseDisplayWidget = new VisualDatabaseDisplayWidget(this, databaseModel);
    setCentralWidget(visualDatabaseDisplayWidget);

    cardInfoDock = new QDockWidget(this);
    cardInfoDock->setObjectName("visualDatabaseCardInfoDock");
    cardInfoDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    auto *contents = new QWidget(cardInfoDock);
    auto *layout = new QVBoxLayout(contents);
    selectionHint = new QLabel(contents);
    selectionHint->setWordWrap(true);
    cardInfo = new CardInfoFrameWidget(contents);
    cardInfo->setObjectName("visualDatabaseCardInfo");
    layout->addWidget(selectionHint);
    layout->addWidget(cardInfo);
    cardInfoDock->setWidget(contents);

    viewMenu = new QMenu(this);
    viewMenu->addAction(cardInfoDock->toggleViewAction());
    viewMenu->addSeparator();
    resetLayoutAction = viewMenu->addAction(QString(), this, &TabVisualDatabaseDisplay::resetLayout);
    addTabMenu(viewMenu);
    resetLayout();
    restoreState(SettingsCache::instance().layouts().getVisualDatabaseDisplayLayoutState());

    connect(visualDatabaseDisplayWidget, &VisualDatabaseDisplayWidget::edhrecRequested, this,
            &TabVisualDatabaseDisplay::openEdhrecTab);
    connect(visualDatabaseDisplayWidget, &VisualDatabaseDisplayWidget::cardSelectedDatabaseDisplay, this,
            &TabVisualDatabaseDisplay::inspectCard);
    connect(visualDatabaseDisplayWidget, &VisualDatabaseDisplayWidget::cardInfoRequested, this,
            &TabVisualDatabaseDisplay::inspectCard);
    connect(visualDatabaseDisplayWidget, &VisualDatabaseDisplayWidget::displayModeChanged, this, [this] {
        emit tabTextChanged(this, getTabText());
        if (!visualDatabaseDisplayWidget->isVisualDisplayMode() && !cardInfo->getCard().isEmpty()) {
            auto *view = visualDatabaseDisplayWidget->getDatabaseView();
            const auto matches = view->model()->match(view->model()->index(0, CardDatabaseModel::NameColumn),
                                                     Qt::DisplayRole, cardInfo->getCard().getName(), 1,
                                                     Qt::MatchExactly);
            if (!matches.isEmpty()) {
                view->setCurrentIndex(matches.first());
                view->scrollTo(matches.first());
            }
        }
    });

    // An invalid table selection does not emit CardDatabaseView::cardChanged.
    // Clear the inspector explicitly so an empty result never shows stale rules.
    auto *view = visualDatabaseDisplayWidget->getDatabaseView();
    connect(view->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            [this](const QModelIndex &current) {
                if (!visualDatabaseDisplayWidget->isVisualDisplayMode() && !current.isValid()) {
                    inspectCard(ExactCard());
                }
            });
    auto *model = visualDatabaseDisplayWidget->getDatabaseDisplayModel();
    connect(model, &CardDatabaseDisplayModel::modelDirty, this, [this, model] {
        if (model->rowCount() == 0) {
            inspectCard(ExactCard());
        }
    });
    connect(CardDatabaseManager::getInstance(), &CardDatabase::cardDatabaseReset, this,
            [this] { inspectCard(ExactCard()); });

    inspectCard(ExactCard());
    TabVisualDatabaseDisplay::retranslateUi();
}

void TabVisualDatabaseDisplay::inspectCard(const ExactCard &card)
{
    cardInfo->setCard(card);
    selectionHint->setVisible(card.isEmpty());
}

void TabVisualDatabaseDisplay::resetLayout()
{
    cardInfoDock->setFloating(false);
    addDockWidget(Qt::RightDockWidgetArea, cardInfoDock);
    cardInfoDock->show();
    resizeDocks({cardInfoDock}, {300}, Qt::Horizontal);
}

void TabVisualDatabaseDisplay::hideEvent(QHideEvent *event)
{
    SettingsCache::instance().layouts().setVisualDatabaseDisplayLayoutState(saveState());
    Tab::hideEvent(event);
}

void TabVisualDatabaseDisplay::retranslateUi()
{
    visualDatabaseDisplayWidget->retranslateUi();
    cardInfo->retranslateUi();
    cardInfoDock->setWindowTitle(tr("Card Info"));
    viewMenu->setTitle(tr("&View"));
    resetLayoutAction->setText(tr("Reset layout"));
    selectionHint->setText(tr("Select a card to inspect its rules and printing."));
    emit tabTextChanged(this, getTabText());
}

void TabVisualDatabaseDisplay::openEdhrecTab(const CardInfoPtr &info, bool isCommander) const
{
    getTabSupervisor()->addEdhrecTab(info, isCommander);
}
