/**
 * @file card_completer_utils.h
 * @ingroup UtilityWidgets
 */
//! \todo Document this file.

#ifndef CARD_COMPLETER_UTILS_H
#define CARD_COMPLETER_UTILS_H

class CardCompleterProxyModel;
class CardDatabaseDisplayModel;
class CardSearchModel;
class QCompleter;
class QLineEdit;
class QObject;
class QStringListModel;

struct CardCompleterSetup
{
    CardSearchModel *searchModel;
    CardCompleterProxyModel *proxyModel;
    QCompleter *completer;
};

CardCompleterSetup
createCardCompleter(CardDatabaseDisplayModel *displayModel, QObject *parent, int maxVisibleItems = 10);

void connectCardCompleterSearch(QLineEdit *edit, const CardCompleterSetup &setup);

QCompleter *createMentionCompleter(QStringListModel *model, QObject *parent);

#endif // CARD_COMPLETER_UTILS_H
