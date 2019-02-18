#include "player.h"
#include "arrowitem.h"
#include "carddatabase.h"
#include "carditem.h"
#include "cardlist.h"
#include "cardzone.h"
#include "color.h"
#include "counter_general.h"
#include "deck_loader.h"
#include "dlg_create_token.h"
#include "expression.h"
#include "gamescene.h"
#include "handcounter.h"
#include "handzone.h"
#include "main.h"
#include "pilezone.h"
#include "playertarget.h"
#include "settingscache.h"
#include "stackzone.h"
#include "tab_game.h"
#include "tablezone.h"
#include "thememanager.h"
#include "zoneviewwidget.h"
#include "zoneviewzone.h"
#include <QDebug>
#include <QMenu>
#include <QPainter>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSignalMapper>

#include "pb/command_attach_card.pb.h"
#include "pb/command_change_zone_properties.pb.h"
#include "pb/command_create_token.pb.h"
#include "pb/command_draw_cards.pb.h"
#include "pb/command_flip_card.pb.h"
#include "pb/command_game_say.pb.h"
#include "pb/command_move_card.pb.h"
#include "pb/command_mulligan.pb.h"
#include "pb/command_reveal_cards.pb.h"
#include "pb/command_roll_die.pb.h"
#include "pb/command_set_card_attr.pb.h"
#include "pb/command_set_card_counter.pb.h"
#include "pb/command_shuffle.pb.h"
#include "pb/command_undo_draw.pb.h"
#include "pb/context_move_card.pb.h"
#include "pb/context_undo_draw.pb.h"
#include "pb/serverinfo_player.pb.h"
#include "pb/serverinfo_user.pb.h"
#include "pb/serverinfo_zone.pb.h"

void Player::runActionCode(QString code, CardItem *card) {
    Expression exp(0);

    exp.addFunction("AddCounter", [=](QVariantList args) {
        int counterId = args.size() > 0 ? args[0].toInt() : 0;
        int amt = args.size() > 1 ? args[1].toInt() : 1;

        if (card->getCounters().value(counterId, 0) < MAX_COUNTERS_ON_CARD) {
            Command_SetCardCounter cmd;
            cmd.set_zone(card->getZone()->getName().toStdString());
            cmd.set_card_id(card->getId());
            cmd.set_counter_id(counterId);
            cmd.set_counter_value(card->getCounters().value(counterId, 0) + amt);
            sendGameCommand(cmd);
        }
        return card->getCounters().value(counterId, 0);
    });

    exp.addFunction("Tap", [=](QVariantList args) {
        Command_SetCardAttr cmd;
        cmd.set_zone(card->getZone()->getName().toStdString());
        cmd.set_card_id(card->getId());
        cmd.set_attribute(AttrTapped);
        cmd.set_attr_value("1");
        sendGameCommand(cmd);
        return QVariant();
    });

    exp.addFunction("Untap", [=](QVariantList args) {
        Command_SetCardAttr cmd;
        cmd.set_zone(card->getZone()->getName().toStdString());
        cmd.set_card_id(card->getId());
        cmd.set_attribute(AttrTapped);
        cmd.set_attr_value("0");
        sendGameCommand(cmd);
        return QVariant();
    });

    exp.addFunction("Life", [=](QVariantList args) {
        int amt = args.size() > 0 ? args[0].toInt() : 1;
        auto ctr = counters.value(0);
        int value = ctr->getValue() + amt;
        if (amt != 0) ctr->setCounterTo(value);
        return value;
    });

    exp.parse(QString("{ %1 }").arg(code));
}