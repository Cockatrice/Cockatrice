#include "card_localization.h"

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
    if (lang == "zhs") {
        return QStringLiteral("简体中文");
    }
    if (lang == "zht") {
        return QStringLiteral("繁體中文");
    }
    const QString nativeName = QLocale(lang).nativeLanguageName();
    return nativeName.isEmpty() ? lang : nativeName;
}
} // namespace CardLocalization