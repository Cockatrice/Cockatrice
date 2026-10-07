/**
 * @file token_info.h
 * @ingroup GameLogicPlayers
 */
//! \todo Document this file.

#ifndef COCKATRICE_TOKEN_INFO_H
#define COCKATRICE_TOKEN_INFO_H

#include <QString>

struct TokenInfo
{
    QString name;
    QString color;
    QString pt;
    QString annotation;
    bool destroy = true;
    bool faceDown = false;
    QString providerId;
};

#endif // COCKATRICE_TOKEN_INFO_H
