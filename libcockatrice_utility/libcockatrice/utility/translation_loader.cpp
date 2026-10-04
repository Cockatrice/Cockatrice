#include "translation_loader.h"

#include <QChar>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLatin1Char>
#include <QLibraryInfo>
#include <QMessageLogger>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSet>
#include <QSharedPointer>
#include <QStringList>
#include <QStringLiteral>
#include <QTranslator>
#include <QtVersionChecks>
#include <algorithm>
#include <initializer_list>

// Kept under the historical category name so existing QT_LOGGING_RULES filters keep working.
Q_LOGGING_CATEGORY(TranslationLoaderLog, "qt_translator")

QList<QString> TranslationLoader::qtTranslationCandidates()
{
    QStringList candidates;

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    // Qt 6.8 exposes every path listed in qt.conf, path() only returns the first one.
    candidates << QLibraryInfo::paths(QLibraryInfo::TranslationsPath);
#else
    candidates << QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#endif

    // A relocated Qt can report a prefix-relative path that does not exist, so probe the usual
    // install layouts as well instead of trusting that single report.
    const QString prefix = QLibraryInfo::path(QLibraryInfo::PrefixPath);
    const QString dataPath = QLibraryInfo::path(QLibraryInfo::DataPath);
    candidates << prefix + QStringLiteral("/share/qt6/translations")
               << prefix + QStringLiteral("/share/qt/translations") << prefix + QStringLiteral("/translations")
               << dataPath + QStringLiteral("/translations");

    QList<QString> uniqueCandidates;
    for (const QString &candidate : candidates) {
        if (!candidate.isEmpty() && !uniqueCandidates.contains(candidate)) {
            uniqueCandidates << candidate;
        }
    }
    return uniqueCandidates;
}

QList<QString> TranslationLoader::qtTranslationPaths()
{
    QList<QString> existingPaths;
    for (const QString &candidate : qtTranslationCandidates()) {
        if (QFileInfo::exists(candidate)) {
            existingPaths << candidate;
        }
    }
    return existingPaths;
}

QList<QString> TranslationLoader::applicationTranslationPaths(const QString &_installedPath)
{
    // A build tree keeps the .qm files next to the executable, the installed layout puts them in
    // _installedPath. Searching both keeps a developer build translated as well.
    const QString executableDirectory = QCoreApplication::applicationDirPath();

    QList<QString> paths;
    for (const QString &path : {_installedPath, executableDirectory}) {
        if (!path.isEmpty() && !paths.contains(path)) {
            paths << path;
        }
    }
    return paths;
}

QList<QString> TranslationLoader::availableLanguages(const QString &_prefix, const QList<QString> &_directories)
{
    const QRegularExpression filePattern(
        QRegularExpression::anchoredPattern(QRegularExpression::escape(_prefix) + QStringLiteral("_(.*)\\.qm")));

    QSet<QString> languages;
    for (const QString &directory : _directories) {
        if (directory.isEmpty()) {
            continue;
        }
        const QDir dir(directory);
        for (const QString &fileName : dir.entryList(QDir::Files)) {
            const auto match = filePattern.match(fileName);
            if (match.hasMatch()) {
                languages.insert(match.captured(1));
            }
        }
    }

    QList<QString> sortedLanguages(languages.cbegin(), languages.cend());
    std::sort(sortedLanguages.begin(), sortedLanguages.end());
    return sortedLanguages;
}

TranslationLoadResult
TranslationLoader::loadFrom(QTranslator &_translator, const QString &_nameHint, const QList<QString> &_directories)
{
    bool anyDirectoryExists = false;

    for (const QString &directory : _directories) {
        if (directory.isEmpty() || !QFileInfo::exists(directory)) {
            continue;
        }
        anyDirectoryExists = true;
        if (_translator.load(_nameHint, directory)) {
            return TranslationLoadResult::Loaded;
        }
    }

    return anyDirectoryExists ? TranslationLoadResult::Missing : TranslationLoadResult::NoDirectory;
}

TranslationLoadResult TranslationLoader::loadQt(QTranslator &_translator, const QString &_lang)
{
    // An unset language is the default for English, which has no translation file to begin with.
    if (_lang.isEmpty()) {
        return TranslationLoadResult::Missing;
    }

    const QString nameHint = QStringLiteral("qt_") + _lang;
    const auto result = loadFrom(_translator, nameHint, qtTranslationPaths());

    switch (result) {
        case TranslationLoadResult::Loaded:
            qCInfo(TranslationLoaderLog) << "Loaded Qt translation" << nameHint << "at" << _translator.filePath();
            break;
        case TranslationLoadResult::Missing:
            qCDebug(TranslationLoaderLog)
                << "No Qt translation" << nameHint << "in" << QStringList(qtTranslationPaths());
            break;
        case TranslationLoadResult::NoDirectory:
            qCWarning(TranslationLoaderLog)
                << "No Qt translations directory exists, looked in" << QStringList(qtTranslationPaths());
            break;
    }

    return result;
}

TranslationLoadResult TranslationLoader::loadApplication(QTranslator &_translator,
                                                         const QString &_prefix,
                                                         const QString &_lang,
                                                         const QList<QString> &_directories)
{
    if (_lang.isEmpty()) {
        return TranslationLoadResult::Missing;
    }

    const QString nameHint = _prefix + QLatin1Char('_') + _lang;
    const auto result = loadFrom(_translator, nameHint, _directories);

    switch (result) {
        case TranslationLoadResult::Loaded:
            qCInfo(TranslationLoaderLog) << "Loaded" << _prefix << "translation" << nameHint << "at"
                                         << _translator.filePath();
            break;
        case TranslationLoadResult::Missing:
            qCDebug(TranslationLoaderLog)
                << "No" << _prefix << "translation" << nameHint << "in" << QStringList(_directories);
            break;
        case TranslationLoadResult::NoDirectory:
            qCWarning(TranslationLoaderLog)
                << "No translation directory exists, looked in" << QStringList(_directories);
            break;
    }

    return result;
}
