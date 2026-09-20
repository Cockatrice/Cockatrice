#ifndef CARD_LOCALIZATION_H
#define CARD_LOCALIZATION_H

#include <QString>
#include <QStringList>

/**
 * @brief The card languages card search should run against.
 */
enum class SearchLanguageMode
{
    English,  ///< Only search the English card names and texts.
    Selected, ///< Search the selected card language (untranslated cards still match in English).
    Both      ///< Search both the English and the selected card language names and texts.
};

/**
 * @brief The card language and matching mode searches run against.
 *
 * Bundles the card language code configured in the settings with the
 * SearchLanguageMode, so entry points take one value instead of two related
 * parameters.
 */
struct CardSearchLanguage
{
    QString language; ///< Card language code (e.g. "de"); empty means the English fallback.
    SearchLanguageMode mode = SearchLanguageMode::English; ///< How the language participates in the search.

    /**
     * @brief Whether only the English card data is searched.
     *
     * @return True when no card language is selected or English itself is selected.
     */
    [[nodiscard]] bool isEnglishOnly() const
    {
        return language.isEmpty() || language == QLatin1String("en");
    }

    bool operator==(const CardSearchLanguage &) const = default;
    bool operator!=(const CardSearchLanguage &) const = default;
};

/**
 * @namespace CardLocalization
 * @ingroup Cards
 *
 * @brief Shared language metadata for localized card text and images.
 *
 * Lists the language codes Cockatrice can display localized card data for and
 * provides human-readable names. The list is shared between Oracle (which
 * imports the selected language's card data) and the client settings UI (which
 * offers the language choice).
 */
namespace CardLocalization
{
/**
 * @brief Language codes for which localized card data can be imported/displayed.
 *
 * Matches the languages Scryfall can serve localized card images for. "en" is
 * always available as the default/fallback and is not listed here.
 *
 * @return The list of supported language codes.
 */
[[nodiscard]] const QStringList &supportedLanguages();

/**
 * @brief Human-readable name for a language code.
 *
 * Follows the same "native name (English name)" format the UI language list
 * uses (e.g. "日本語 (Japanese)"), so the English fallback is always visible.
 *
 * @param lang Language code (e.g. "de", "ja", "zhs").
 * @return The language's native name with its English name in parentheses, or
 *         the code itself if it cannot be resolved.
 */
[[nodiscard]] QString languageDisplayName(const QString &lang);
} // namespace CardLocalization

#endif // CARD_LOCALIZATION_H