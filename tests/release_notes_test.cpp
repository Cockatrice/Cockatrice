#include "client/network/update/client/release_notes.h"

#include "gtest/gtest.h"

namespace
{
const QString STABLE_BODY = QStringLiteral(R"BODY(<!-- this template comes from .ci/release_template.md -->

<pre>
Available pre-compiled binaries for installation:
</pre>

## General Notes

We're pleased to announce the newest official release: <kbd>2.10.0</kbd>

## Changelog
<!--
This list is generated and should be moved to their respective header.
-->

<!-- app-notes:start -->
<!-- Highlights of the release -->
### 🔖 Highlights:
- The deck editor now remembers the last opened deck.

### ✨ New Features:
- Added a keyboard shortcut for the sideboard.

### 🐛 Fixed Bugs / Resolved Issues:
- Fixed a crash when closing a game during sideboarding.
<!-- app-notes:end -->

<!-- Complete list of changes (foldable) -->
<details>
<summary>
<b>Show all changes</b> (123 commits)
</summary>

### User Interface
- commit: polish the toolbar

</details>

## Translations
- Thanks for over 300 people contributing.
)BODY");

const QString BETA_BODY = QStringLiteral(R"BODY(42 commits have been included over the previous beta release

<details>
<summary><b>show changes</b></summary>

### Under the Hood
- commit: bump the dependency

</details>
)BODY");

TEST(ReleaseNotes, MarkerBlockWinsOverChangelog)
{
    const QString notes = ReleaseNotes::curated(STABLE_BODY);

    EXPECT_TRUE(notes.contains(QStringLiteral("remembers the last opened deck")));
    EXPECT_TRUE(notes.contains(QStringLiteral("keyboard shortcut for the sideboard")));
    EXPECT_TRUE(notes.contains(QStringLiteral("Fixed a crash when closing a game")));
    EXPECT_FALSE(notes.contains(QStringLiteral("Show all changes")));
    EXPECT_FALSE(notes.contains(QStringLiteral("polish the toolbar")));
    EXPECT_FALSE(notes.contains(QStringLiteral("General Notes")));
    EXPECT_FALSE(notes.contains(QStringLiteral("Translations")));
}

TEST(ReleaseNotes, EmptyMarkerBlockYieldsNoNotes)
{
    const QString body = QStringLiteral(R"BODY(<!-- app-notes:start -->
<!-- app-notes:end -->

<details>
<summary><b>Show all changes</b></summary>

- commit: something user-visible

</details>
)BODY");

    EXPECT_TRUE(ReleaseNotes::curated(body).isEmpty());
}

TEST(ReleaseNotes, ChangelogSectionWithoutMarkers)
{
    const QString body = QStringLiteral(R"BODY(## General Notes

Some prose.

## Changelog

### 🔖 Highlights:
- A curated highlight.

### ✨ New Features:
- A curated feature.

<details>
<summary><b>Show all changes</b></summary>

- commit: not for the app

</details>

## Translations
- Unrelated section.
)BODY");

    const QString notes = ReleaseNotes::curated(body);

    EXPECT_TRUE(notes.contains(QStringLiteral("A curated highlight.")));
    EXPECT_TRUE(notes.contains(QStringLiteral("A curated feature.")));
    EXPECT_FALSE(notes.contains(QStringLiteral("not for the app")));
    EXPECT_FALSE(notes.contains(QStringLiteral("Unrelated section.")));
}

TEST(ReleaseNotes, LegacyDetailsBody)
{
    const QString notes = ReleaseNotes::curated(BETA_BODY);

    EXPECT_TRUE(notes.contains(QStringLiteral("bump the dependency")));
    EXPECT_FALSE(notes.contains(QStringLiteral("show changes")));
    EXPECT_FALSE(notes.contains(QStringLiteral("42 commits")));
}

TEST(ReleaseNotes, AlertBlockBecomesBoldLeadIn)
{
    const QString body = QStringLiteral(R"BODY(<!-- app-notes:start -->
> [!IMPORTANT]
> **With this release, we no longer provide a binary for Windows 8.**
<!-- app-notes:end -->
)BODY");

    const QString notes = ReleaseNotes::curated(body);

    EXPECT_TRUE(notes.contains(QStringLiteral("> **Important:**")));
    EXPECT_TRUE(notes.contains(QStringLiteral("no longer provide a binary for Windows 8")));
    EXPECT_FALSE(notes.contains(QStringLiteral("[!IMPORTANT]")));
}

TEST(ReleaseNotes, HeadingsWithoutContentAreDropped)
{
    const QString body = QStringLiteral(R"BODY(<!-- app-notes:start -->
### 🔖 Highlights:
### ✨ New Features:
- A real feature.
<!-- app-notes:end -->
)BODY");

    const QString notes = ReleaseNotes::curated(body);

    EXPECT_FALSE(notes.contains(QStringLiteral("Highlights")));
    EXPECT_TRUE(notes.contains(QStringLiteral("New Features")));
    EXPECT_TRUE(notes.contains(QStringLiteral("A real feature.")));
}

TEST(ReleaseNotes, CrLfIsNormalized)
{
    const QString body = QStringLiteral(
        "<!-- app-notes:start -->\r\n### Highlights:\r\n- Windows line ending.\r\n<!-- app-notes:end -->\r\n");

    const QString notes = ReleaseNotes::curated(body);

    EXPECT_TRUE(notes.contains(QStringLiteral("Windows line ending.")));
    EXPECT_FALSE(notes.contains(QLatin1Char('\r')));
}

TEST(ReleaseNotes, MalformedBodyHasNoNotes)
{
    EXPECT_TRUE(ReleaseNotes::curated(QStringLiteral("Just prose, no sections at all.")).isEmpty());
    EXPECT_TRUE(ReleaseNotes::curated(QStringLiteral("<details>never closed")).isEmpty());
    EXPECT_TRUE(ReleaseNotes::curated(QStringLiteral("<!-- app-notes:start -->never closed")).isEmpty());
}
} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
