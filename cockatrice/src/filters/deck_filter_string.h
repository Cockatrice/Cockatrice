/**
 * @file deck_filter_string.h
 * @ingroup DeckStorageWidgets
 * @brief TODO: Document this.
 */

#ifndef DECK_FILTER_STRING_H
#define DECK_FILTER_STRING_H

#include "../interface/widgets/visual_deck_storage/deck_preview/deck_preview_widget.h"

#include <QLoggingCategory>
#include <QString>
#include <functional>

inline Q_LOGGING_CATEGORY(DeckFilterStringLog, "deck_filter_string");

/**
 * Extra info relevant to filtering that isn't present in the DeckPreviewWidget
 */
struct ExtraDeckSearchInfo
{
    /**
     * The relative filepath starting from the deck folder
     */
    QString relativeFilePath;
};

typedef std::function<bool(const DeckPreviewWidget *, const ExtraDeckSearchInfo &)> DeckFilter;

class DeckFilterString
{
public:
    DeckFilterString();
    explicit DeckFilterString(const QString &expr);
    bool check(const DeckPreviewWidget *deck, const ExtraDeckSearchInfo &info) const
    {
        return filter(deck, info);
    }

    [[nodiscard]] bool valid() const
    {
        return _error.isEmpty();
    }

    QString error()
    {
        return _error;
    }

private:
    QString _error;
    DeckFilter filter;
};
#endif // DECK_FILTER_STRING_H
