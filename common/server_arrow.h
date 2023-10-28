#ifndef SERVER_ARROW_H
#define SERVER_ARROW_H

#include "pb/color.pb.h"

class Server_Card;
class Server_ArrowTarget;
class ServerInfo_Arrow;

class Server_Arrow
{
private:
    int id;
    Server_Card *startCard;
    Server_ArrowTarget *targetItem;
    color arrowColor;

public:
    Server_Arrow(int _id, Server_Card *_startCard, Server_ArrowTarget *_targetItem, const color &_arrowColor);
    int getId() const
    {
        return id;
    }
    Server_Card *getStartCard() const
    {
        return startCard;
    }
    void setStartCard(Server_Card *startCard_)
    {
        startCard = startCard_;
    }
    Server_ArrowTarget *getTargetItem() const
    {
        return targetItem;
    }
    void setTargetItem(Server_ArrowTarget *targetItem_)
    {
        targetItem = targetItem_;
    }
    const color &getColor() const
    {
        return arrowColor;
    }

    void getInfo(ServerInfo_Arrow *info);
};

#endif
