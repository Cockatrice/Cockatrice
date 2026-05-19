#include "custom_zone_menu.h"

#include "../player_logic.h"

CustomZoneMenu::CustomZoneMenu(PlayerLogic *_player) : player(_player)
{
    menuAction()->setVisible(false);

    connect(player, &PlayerLogic::clearCustomZonesMenu, this, &CustomZoneMenu::clearCustomZonesMenu);
    connect(player, &PlayerLogic::addViewCustomZoneActionToCustomZoneMenu, this,
            &CustomZoneMenu::addViewCustomZoneActionToCustomZoneMenu);

    retranslateUi();
}

void CustomZoneMenu::retranslateUi()
{
    setTitle(tr("C&ustom Zones"));

    if (player->getPlayerInfo()->getLocalOrJudge()) {

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
            [zoneName, this]() { player->getGameScene()->toggleZoneView(player, zoneName, -1); });
}