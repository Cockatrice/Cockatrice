/**
 * @file release_notes.h
 * @ingroup ClientUpdate
 *
 * Turns a GitHub release body into the short, maintainer-curated notes that the
 * update dialog shows. The full body stays on GitHub; only the part a maintainer
 * marked for the app is rendered here.
 */

#ifndef RELEASE_NOTES_H
#define RELEASE_NOTES_H

#include <QString>

namespace ReleaseNotes
{
/**
 * Returns the curated portion of a release body as Markdown, or an empty string
 * when the release carries no usable curated notes.
 *
 * Resolution order:
 *   1. the block between `<!-- app-notes:start -->` and `<!-- app-notes:end -->`;
 *   2. otherwise the `## Changelog` section;
 *   3. otherwise the content of the first `<details>` block, without its summary.
 *
 * A release whose marker block is present but empty yields an empty string: the
 * dialog then shows the version details and a link instead of commit subjects.
 */
QString curated(const QString &body);
} // namespace ReleaseNotes

#endif
