#ifndef TRANSLATION_LOADER_H
#define TRANSLATION_LOADER_H

#include <QList>
#include <QLoggingCategory>
#include <QString>

class QTranslator;

Q_DECLARE_LOGGING_CATEGORY(TranslationLoaderLog)

/** @brief The outcome of a translation load attempt. */
enum class TranslationLoadResult
{
    Loaded,     /**< A translation file was found and loaded into the translator. */
    Missing,    /**< The search directories exist but hold no file for this language. */
    NoDirectory /**< Not a single search directory exists, so no lookup could be attempted. */
};

namespace TranslationLoader
{
/**
 * @brief Every directory that may hold Qt's own translations, in order of preference.
 *
 * More than one entry is needed because a relocated Qt (Flatpak, AppImage, a bundled SDK) can report
 * a prefix-relative translations path that does not exist: the KDE runtime reports /usr/translations
 * while its qt_*.qm files live in /usr/share/qt6/translations.
 */
QList<QString> qtTranslationCandidates();

/**
 * @brief The candidates that actually exist, most likely first, so no caller searches a path that is
 *        not there.
 */
QList<QString> qtTranslationPaths();

/**
 * @brief The directories to search for an application's own translation files, in order of preference.
 *
 * @param _installedPath The location the platform installs the translation files in, derived from the
 *        executable's path.
 * @return The installed location, followed by the executable's own directory: a build tree keeps the
 *         .qm files next to the binary instead of in the installed layout, so a developer build would
 *         otherwise find no translations at all.
 */
QList<QString> applicationTranslationPaths(const QString &_installedPath);

/**
 * @brief Loads the first _nameHint + language translation file found in _directories into _translator.
 *
 * QTranslator discards whatever it held before every load, so each translation file needs its own
 * translator; loading a second file into the first one silently drops the first.
 *
 * @param _translator The translator to load into.
 * @param _nameHint The translation file name without its language and suffix, e.g. "qt_de" or "cockatrice_de".
 * @param _directories The directories to search, in order of preference.
 * @return What the search found.
 */
TranslationLoadResult loadFrom(QTranslator &_translator, const QString &_nameHint, const QList<QString> &_directories);

/**
 * @brief Loads Qt's own translation for _lang, which covers the strings of Qt's own dialogs.
 *
 * @param _translator The translator to load into; must not also be used for an application translation.
 * @param _lang The language code, e.g. "de". An empty language loads nothing.
 * @return What the search found.
 */
TranslationLoadResult loadQt(QTranslator &_translator, const QString &_lang);

/**
 * @brief The language codes of the <_prefix>_<lang>.qm files in _directories, sorted by code.
 *
 * Lets a language selector offer exactly the languages that can be loaded.
 */
QList<QString> availableLanguages(const QString &_prefix, const QList<QString> &_directories);

/**
 * @brief Loads the <_prefix>_<_lang>.qm translation file from _directories into _translator.
 *
 * @param _translator The translator to load into; must not also be used for Qt's own translation.
 * @param _prefix The translation file prefix, e.g. "cockatrice" or "oracle".
 * @param _lang The language code, e.g. "de". An empty language loads nothing.
 * @param _directories The directories to search, in order of preference.
 * @return What the search found.
 */
TranslationLoadResult loadApplication(QTranslator &_translator,
                                      const QString &_prefix,
                                      const QString &_lang,
                                      const QList<QString> &_directories);
} // namespace TranslationLoader

#endif // TRANSLATION_LOADER_H
