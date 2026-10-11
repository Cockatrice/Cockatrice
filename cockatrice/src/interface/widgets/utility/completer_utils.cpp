#include "completer_utils.h"

#include "../../../client/settings/cache_settings.h"
#include "card_completer_styler.h"

#include <QAbstractItemView>
#include <QCompleter>
#include <QLineEdit>
#include <QModelIndex>
#include <QObject>
#include <QPainter>
#include <QPalette>
#include <QRegularExpression>
#include <QString>
#include <QStringListModel>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>
#include <libcockatrice/card/card_localization.h>
#include <libcockatrice/models/database/card/card_completer_proxy_model.h>
#include <libcockatrice/models/database/card/card_search_model.h>
#include <libcockatrice/settings/cards_display_settings.h>
#include <qnamespace.h>

namespace
{
/**
 * @brief Paints the mention popup's current row with a visible highlight.
 *
 * A completer popup is a transient window that is usually not the application's
 * active window, so the style paints a selected row using the palette's
 * Inactive highlight group. Most themes set that colour close to the popup
 * background, which hides the current completion entirely. Mirror the active
 * highlight into the inactive group so the selection stays visible.
 */
class MentionCompleterDelegate : public QStyledItemDelegate
{
public:
    explicit MentionCompleterDelegate(QAbstractItemView *view) : QStyledItemDelegate(view), view(view)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt = option;
        opt.showDecorationSelected = true;

        if (view->currentIndex() == index) {
            opt.state |= QStyle::State_HasFocus;
        }

        if (opt.state & QStyle::State_Selected) {
            opt.palette.setColor(QPalette::Inactive, QPalette::Highlight,
                                 opt.palette.color(QPalette::Active, QPalette::Highlight));
            opt.palette.setColor(QPalette::Inactive, QPalette::HighlightedText,
                                 opt.palette.color(QPalette::Active, QPalette::HighlightedText));
        }

        QStyledItemDelegate::paint(painter, opt, index);
    }

private:
    QAbstractItemView *view;
};

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

    // Force the popup to be created so the highlight delegate can be installed;
    // otherwise QCompleter's own delegate hides the selection on most themes.
    QAbstractItemView *popup = completer->popup();
    popup->setItemDelegate(new MentionCompleterDelegate(popup));

    return completer;
}
