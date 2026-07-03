#include "custom_zone_menu.h"

#include "../../game/player/player_logic.h"
#include "../player_graphics_item.h"

CustomZoneMenu::CustomZoneMenu(PlayerGraphicsItem *_player) : player(_player)
{
    menuAction()->setVisible(false);

    connect(player->getLogic(), &PlayerLogic::clearCustomZonesMenu, this, &CustomZoneMenu::clearCustomZonesMenu);
    connect(player->getLogic(), &PlayerLogic::addViewCustomZoneActionToCustomZoneMenu, this,
            &CustomZoneMenu::addViewCustomZoneActionToCustomZoneMenu);

    retranslateUi();
}

void CustomZoneMenu::retranslateUi()
{
    setTitle(tr("C&ustom Zones"));

    if (player->getLogic()->getPlayerInfo()->getLocalOrJudge()) {

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
            [zoneName, this]() { player->getGameScene()->toggleZoneView(player->getLogic(), zoneName, -1); });
}