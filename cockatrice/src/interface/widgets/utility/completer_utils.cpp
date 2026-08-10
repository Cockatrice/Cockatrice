#include "completer_utils.h"

#include "card_completer_styler.h"

#include <QCompleter>
#include <QLineEdit>
#include <QObject>
#include <QRegularExpression>
#include <QStringListModel>
#include <libcockatrice/models/database/card/card_completer_proxy_model.h>
#include <libcockatrice/models/database/card/card_search_model.h>
#include <libcockatrice/models/database/card_database_display_model.h>

CardCompleterSetup createCardCompleter(CardDatabaseDisplayModel *displayModel, QObject *parent, int maxVisibleItems)
{
    auto *searchModel = new CardSearchModel(displayModel, parent);

    auto *proxyModel = new CardCompleterProxyModel(parent);
    proxyModel->setSourceModel(searchModel);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    auto *completer = new QCompleter(proxyModel, parent);
    completer->setCompletionRole(Qt::DisplayRole);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    completer->setMaxVisibleItems(maxVisibleItems);
    CardCompleterStyler::apply(completer);

    return {searchModel, proxyModel, completer};
}

void connectCardCompleterSearch(QLineEdit *edit, const CardCompleterSetup &setup)
{
    QObject::connect(edit, &QLineEdit::textEdited, setup.searchModel, &CardSearchModel::updateSearchResults);
    QObject::connect(edit, &QLineEdit::textEdited, setup.completer, [setup](const QString &text) {
        setup.proxyModel->setFilterRegularExpression(
            QRegularExpression(QRegularExpression::escape(text), QRegularExpression::CaseInsensitiveOption));
        if (!text.isEmpty()) {
            setup.completer->complete();
        }
    });
}

QCompleter *createMentionCompleter(QStringListModel *model, QObject *parent)
{
    auto *completer = new QCompleter(model, parent);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setMaxVisibleItems(5);
    completer->setFilterMode(Qt::MatchStartsWith);
    return completer;
}
