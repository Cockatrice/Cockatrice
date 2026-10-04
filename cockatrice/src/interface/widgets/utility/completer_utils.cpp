#include "completer_utils.h"

#include "../../../client/settings/cache_settings.h"
#include "card_completer_styler.h"

#include <QCompleter>
#include <QLineEdit>
#include <QObject>
#include <QRegularExpression>
#include <QStringListModel>
#include <libcockatrice/card/card_localization.h>
#include <libcockatrice/models/database/card/card_completer_proxy_model.h>
#include <libcockatrice/models/database/card/card_search_model.h>
#include <libcockatrice/models/database/card_database_display_model.h>
#include <libcockatrice/settings/cards_display_settings.h>

namespace
{
void applyCardSearchLanguage(CardSearchModel *searchModel)
{
    const CardsDisplaySettings &cardsDisplay = SettingsCache::instance().cardsDisplay();
    searchModel->setSearchLanguage(CardSearchLanguage{
        cardsDisplay.getCardLang(), static_cast<SearchLanguageMode>(cardsDisplay.getCardSearchLanguage())});
}
} // namespace

CardCompleterSetup createCardCompleter(CardDatabaseDisplayModel *displayModel, QObject *parent, int maxVisibleItems)
{
    auto *searchModel = new CardSearchModel(displayModel, parent);
    applyCardSearchLanguage(searchModel);

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

    auto *cardsDisplay = &SettingsCache::instance().cardsDisplay();
    QObject::connect(cardsDisplay, &CardsDisplaySettings::cardLangChanged, searchModel,
                     [searchModel] { applyCardSearchLanguage(searchModel); });
    QObject::connect(cardsDisplay, &CardsDisplaySettings::cardSearchLanguageChanged, searchModel,
                     [searchModel] { applyCardSearchLanguage(searchModel); });

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
