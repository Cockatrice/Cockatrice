#include "levenshtein.h"

#include <QChar>
#include <QSharedPointer>
#include <algorithm>
#include <vector>

int levenshteinDistance(const QString &s1, const QString &s2)
{
    const qsizetype len1 = s1.size();
    const qsizetype len2 = s2.size();

    if (len1 == 0) {
        return static_cast<int>(len2);
    }
    if (len2 == 0) {
        return static_cast<int>(len1);
    }
    if (s1 == s2) {
        return 0;
    }

    // Only the previous row of the distance matrix is needed to compute the
    // next one, so two rows replace the full len1 x len2 matrix. The card
    // search runs this for every matching card on each keystroke.
    std::vector<int> previous(len2 + 1);
    std::vector<int> current(len2 + 1);

    for (qsizetype j = 0; j <= len2; j++) {
        previous[j] = static_cast<int>(j);
    }

    for (qsizetype i = 1; i <= len1; i++) {
        current[0] = static_cast<int>(i);
        const QChar c1 = s1.at(i - 1);

        for (qsizetype j = 1; j <= len2; j++) {
            const int cost = (c1 == s2.at(j - 1)) ? 0 : 1;
            current[j] = std::min(previous[j] + 1, std::min(current[j - 1] + 1, previous[j - 1] + cost));
        }

        std::swap(previous, current);
    }

    return previous[len2];
}
