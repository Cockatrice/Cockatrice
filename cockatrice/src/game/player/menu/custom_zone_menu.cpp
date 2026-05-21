#include "custom_zone_menu.h"

#include "../player_logic.h"

CustomZoneMenu::CustomZoneMenu(PlayerGraphicsItem *_player) : player(_player)
{
    menuAction()->setVisible(false);

    connect(player->getPlayerLogic(), &PlayerLogic::clearCustomZonesMenu, this, &CustomZoneMenu::clearCustomZonesMenu);
    connect(player->getPlayerLogic(), &PlayerLogic::addViewCustomZoneActionToCustomZoneMenu, this,
            &CustomZoneMenu::addViewCustomZoneActionToCustomZoneMenu);

    retranslateUi();
}

void CustomZoneMenu::retranslateUi()
{
    setTitle(tr("C&ustom Zones"));

    if (player->getPlayerLogic()->getPlayerInfo()->getLocalOrJudge()) {

        for (auto aViewZone : actions()) {
            aViewZone->setText(tr("View custom zone '%1'").arg(aViewZone->data().toString()));
        }
    }
}

void CustomZoneMenu::clearCustomZonesMenu()
{
    clear();
    menuAction()->setVisible(false);
}

void CustomZoneMenu::addViewCustomZoneActionToCustomZoneMenu(QString zoneName)
{
    menuAction()->setVisible(true);
    QAction *aViewZone = addAction(tr("View custom zone '%1'").arg(zoneName));
    aViewZone->setData(zoneName);
    connect(aViewZone, &QAction::triggered, this,
            [zoneName, this]() { player->getGameScene()->toggleZoneView(player->getPlayerLogic(), zoneName, -1); });
}