#include "gtest/gtest.h"
#include <QCoreApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QSet>
#include <QString>
#include <QTemporaryDir>
#include <QTranslator>
#include <libcockatrice/utility/translation_loader.h>

// The directory holding translation_loader_test_de.qm, compiled from translation_loader_test.ts by the
// build. QTranslator rejects anything that is not a real .qm file, so a stub cannot stand in here.
#ifndef TRANSLATION_LOADER_TEST_DIR
#define TRANSLATION_LOADER_TEST_DIR ""
#endif

namespace
{
const QString testContext = QStringLiteral("TranslationLoaderTest");
const QString firstMessage = QStringLiteral("Load into the first translator");
const QString secondMessage = QStringLiteral("Load into the second translator");
const QString germanFirstMessage = QStringLiteral("In den ersten Übersetzer laden");
const QString germanSecondMessage = QStringLiteral("In den zweiten Übersetzer laden");

QString translate(const QTranslator &_translator, const QString &_sourceText)
{
    return _translator.translate(testContext.toUtf8().constData(), _sourceText.toUtf8().constData());
}

QString testTranslationDirectory()
{
    return QString(TRANSLATION_LOADER_TEST_DIR);
}

// GTest's main() creates no QCoreApplication, which the search paths are derived from.
const QCoreApplication &application()
{
    static int argc = 1;
    static char executable[] = "translation_loader_test";
    static char *argv[] = {executable, nullptr};
    static const QCoreApplication app(argc, argv);
    return app;
}
} // namespace

TEST(TranslationLoaderTest, LoadsFromTheFirstDirectoryHoldingTheFile)
{
    QTemporaryDir emptyDir;
    ASSERT_TRUE(emptyDir.isValid());

    QTranslator translator;

    // The first directory is searched but has no file, the second one has.
    EXPECT_EQ(TranslationLoadResult::Loaded,
              TranslationLoader::loadFrom(translator, QStringLiteral("translation_loader_test_de"),
                                          {emptyDir.path(), testTranslationDirectory()}));
    EXPECT_EQ(germanFirstMessage, translate(translator, firstMessage));
}

TEST(TranslationLoaderTest, LoadedTranslatorResolvesTheTranslatedString)
{
    QTranslator translator;

    ASSERT_EQ(TranslationLoadResult::Loaded,
              TranslationLoader::loadFrom(translator, QStringLiteral("translation_loader_test_de"),
                                          {testTranslationDirectory()}));

    EXPECT_EQ(germanFirstMessage, translate(translator, firstMessage));
    EXPECT_EQ(germanSecondMessage, translate(translator, secondMessage));
}

TEST(TranslationLoaderTest, SeparateTranslatorsKeepTheirOwnFiles)
{
    // QTranslator discards its previous contents on every load, so a caller reusing one translator
    // for two files keeps only the last of them. That is why Qt's translation and the application
    // translation are loaded into translators of their own.
    QTranslator firstTranslator;
    QTranslator secondTranslator;

    ASSERT_EQ(TranslationLoadResult::Loaded,
              TranslationLoader::loadFrom(firstTranslator, QStringLiteral("translation_loader_test_de"),
                                          {testTranslationDirectory()}));
    ASSERT_EQ(TranslationLoadResult::Loaded,
              TranslationLoader::loadFrom(secondTranslator, QStringLiteral("translation_loader_test_de"),
                                          {testTranslationDirectory()}));

    EXPECT_FALSE(firstTranslator.isEmpty());
    EXPECT_FALSE(secondTranslator.isEmpty());
    EXPECT_EQ(germanFirstMessage, translate(firstTranslator, firstMessage));
    EXPECT_EQ(germanSecondMessage, translate(secondTranslator, secondMessage));
}

TEST(TranslationLoaderTest, ExistingDirectoryWithoutTheFileIsMissing)
{
    QTemporaryDir emptyDir;
    ASSERT_TRUE(emptyDir.isValid());

    QTranslator translator;

    EXPECT_EQ(TranslationLoadResult::Missing,
              TranslationLoader::loadFrom(translator, QStringLiteral("translation_loader_test_de"), {emptyDir.path()}));
    EXPECT_TRUE(translator.isEmpty());
}

TEST(TranslationLoaderTest, NonexistentDirectoriesAreNotSearched)
{
    QTemporaryDir emptyDir;
    ASSERT_TRUE(emptyDir.isValid());

    QTranslator translator;
    const QString missingDirectory = emptyDir.path() + QStringLiteral("/does-not-exist");

    EXPECT_EQ(
        TranslationLoadResult::NoDirectory,
        TranslationLoader::loadFrom(translator, QStringLiteral("translation_loader_test_de"), {missingDirectory}));
    EXPECT_EQ(TranslationLoadResult::NoDirectory,
              TranslationLoader::loadFrom(translator, QStringLiteral("translation_loader_test_de"), {}));
    EXPECT_TRUE(translator.isEmpty());
}

TEST(TranslationLoaderTest, EmptyDirectoryEntriesAreSkipped)
{
    QTranslator translator;

    EXPECT_EQ(TranslationLoadResult::Loaded,
              TranslationLoader::loadFrom(translator, QStringLiteral("translation_loader_test_de"),
                                          {QString(), testTranslationDirectory()}));
}

TEST(TranslationLoaderTest, LanguageVariantsFallBackToTheBaseLanguage)
{
    QTranslator translator;

    // Users pick languages such as en_US or zh-Hans that need not have a translation file of their
    // own; a language without one falls back to the file without its country or script part.
    EXPECT_EQ(TranslationLoadResult::Loaded,
              TranslationLoader::loadFrom(translator, QStringLiteral("translation_loader_test_de_CH"),
                                          {testTranslationDirectory()}));
}

TEST(TranslationLoaderTest, QtTranslationCandidatesCoverTheUsualInstallLayouts)
{
    const QList<QString> candidates = TranslationLoader::qtTranslationCandidates();

    EXPECT_FALSE(candidates.isEmpty());
    for (const QString &candidate : candidates) {
        EXPECT_FALSE(candidate.isEmpty());
    }
    EXPECT_EQ(candidates.size(), QSet<QString>(candidates.cbegin(), candidates.cend()).size());

    // A relocated Qt can report a prefix-relative translations path that does not exist (the KDE
    // runtime reports /usr/translations while its files live in /usr/share/qt6/translations), so the
    // layout Qt actually installs into has to be among the candidates.
    const QString prefix = QLibraryInfo::path(QLibraryInfo::PrefixPath);
    EXPECT_TRUE(candidates.contains(prefix + QStringLiteral("/share/qt6/translations")));
}

TEST(TranslationLoaderTest, QtTranslationPathsOnlyHoldExistingDirectories)
{
    const QList<QString> candidates = TranslationLoader::qtTranslationCandidates();
    const QList<QString> paths = TranslationLoader::qtTranslationPaths();

    for (const QString &path : paths) {
        EXPECT_FALSE(path.isEmpty());
        EXPECT_TRUE(QDir(path).exists()) << path.toStdString();
    }

    EXPECT_EQ(paths.size(), QSet<QString>(paths.cbegin(), paths.cend()).size());

    // Every candidate that exists is searched, and nothing else is.
    for (const QString &candidate : candidates) {
        if (QDir(candidate).exists()) {
            EXPECT_TRUE(paths.contains(candidate)) << candidate.toStdString();
        }
    }
}

TEST(TranslationLoaderTest, LoadApplicationUsesThePrefixAndTheLanguage)
{
    QTranslator translator;

    ASSERT_EQ(TranslationLoadResult::Loaded,
              TranslationLoader::loadApplication(translator, QStringLiteral("translation_loader_test"),
                                                 QStringLiteral("de"), {testTranslationDirectory()}));
    EXPECT_EQ(germanSecondMessage, translate(translator, secondMessage));
}

TEST(TranslationLoaderTest, ABuildTreeFindsTheTranslationsNextToTheExecutable)
{
    // The regression this guards: a build tree keeps the .qm files next to the executable rather than
    // in the installed layout, so a developer build loaded no translation and offered no language.
    // This test binary's .qm file sits in its own directory, just like cockatrice's ones do.
    const QString executableDirectory = application().applicationDirPath();
    const QList<QString> paths = TranslationLoader::applicationTranslationPaths(
        executableDirectory + QStringLiteral("/../share/cockatrice/translations"));

    ASSERT_EQ(2, paths.size());
    EXPECT_EQ(executableDirectory + QStringLiteral("/../share/cockatrice/translations"), paths.at(0));
    EXPECT_EQ(executableDirectory, paths.at(1));

    const QList<QString> languages =
        TranslationLoader::availableLanguages(QStringLiteral("translation_loader_test"), paths);
    EXPECT_EQ(QStringList({QStringLiteral("de")}), QStringList(languages.cbegin(), languages.cend()));
}

TEST(TranslationLoaderTest, ApplicationTranslationPathsHoldNoDuplicates)
{
    const QString executableDirectory = application().applicationDirPath();

    EXPECT_EQ(1, TranslationLoader::applicationTranslationPaths(executableDirectory).size());

    // Without an installed layout to search there is nothing left but the executable's directory.
    const QList<QString> withoutInstalledPath = TranslationLoader::applicationTranslationPaths(QString());
    ASSERT_EQ(1, withoutInstalledPath.size());
    EXPECT_EQ(executableDirectory, withoutInstalledPath.at(0));
}

TEST(TranslationLoaderTest, AvailableLanguagesListsTheTranslationsThatCanBeLoaded)
{
    const QList<QString> languages =
        TranslationLoader::availableLanguages(QStringLiteral("translation_loader_test"), {testTranslationDirectory()});

    EXPECT_EQ(QStringList({QStringLiteral("de")}), QStringList(languages.cbegin(), languages.cend()));

    // Files of another application and files without a language code are not languages.
    EXPECT_TRUE(TranslationLoader::availableLanguages(QStringLiteral("some_other_app"), {testTranslationDirectory()})
                    .isEmpty());
    EXPECT_TRUE(TranslationLoader::availableLanguages(QStringLiteral("translation_loader_test"), {}).isEmpty());
}

TEST(TranslationLoaderTest, LoadApplicationReportsAMissingTranslationDirectory)
{
    QTemporaryDir emptyDir;
    ASSERT_TRUE(emptyDir.isValid());

    QTranslator translator;

    EXPECT_EQ(TranslationLoadResult::NoDirectory,
              TranslationLoader::loadApplication(translator, QStringLiteral("translation_loader_test"),
                                                 QStringLiteral("de"),
                                                 {emptyDir.path() + QStringLiteral("/translations")}));
}

TEST(TranslationLoaderTest, UnsetLanguageLoadsNothing)
{
    QTranslator translator;

    // An unset language is the default for English, which has no translation file to begin with:
    // nothing is searched for and the translator is left untouched.
    EXPECT_EQ(TranslationLoadResult::Missing,
              TranslationLoader::loadApplication(translator, QStringLiteral("translation_loader_test"), QString(),
                                                 {testTranslationDirectory()}));
    EXPECT_EQ(TranslationLoadResult::Missing, TranslationLoader::loadQt(translator, QString()));
    EXPECT_TRUE(translator.isEmpty());
    EXPECT_TRUE(translator.filePath().isEmpty());
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
