#include "card_localization.h"

#include <QHash>
#include <QLocale>
#include <QStringList>

namespace CardLocalization
{
const QStringList &supportedLanguages()
{
    static const QStringList languages = {"cs", "de", "es", "fr", "it", "ja", "ko", "pt", "ru", "zhs", "zht", "he"};
    return languages;
}

QString languageDisplayName(const QString &lang)
{
    static const QHash<QString, QString> displayNames = {
        {"cs", "Česky (Czech)"},
        {"de", "Deutsch (German)"},
        {"es", "Español (Spanish)"},
        {"fr", "Français (French)"},
        {"it", "Italiano (Italian)"},
        {"ja", "日本語 (Japanese)"},
        {"ko", "한국어 (Korean)"},
        {"pt", "Português (Portuguese)"},
        {"ru", "Русский (Russian)"},
        {"he", "עברית (Hebrew)"},
        {"zhs", "简体中文 (Chinese Simplified)"},
        {"zht", "繁體中文 (Chinese Traditional)"},
    };
    const QString displayName = displayNames.value(lang);
    if (!displayName.isEmpty()) {
        return displayName;
    }
    const QString nativeName = QLocale(lang).nativeLanguageName();
    return nativeName.isEmpty() ? lang : nativeName;
}
} // namespace CardLocalization