/**
 * @file filter_preset.h
 * @ingroup CardDatabaseModelFilters
 */

#ifndef FILTER_PRESET_H
#define FILTER_PRESET_H

#include "filter_card.h"

#include <QList>
#include <optional>

/**
 * Value-owned saved filters, independent of the UI and the live filter tree.
 * Parsing validates the entire document before a caller replaces its filters.
 */
struct FilterPreset
{
    struct TermSnapshot
    {
        QString term;
        bool enabled;
    };
    struct TypeSnapshot
    {
        CardFilter::Type type;
        bool enabled;
        QList<TermSnapshot> terms;
    };
    struct AttributeSnapshot
    {
        CardFilter::Attr attr;
        bool enabled;
        QList<TypeSnapshot> types;
    };
    using Snapshot = QList<AttributeSnapshot>;

    Snapshot snapshot;
    // Empty for legacy documents whose color mode is inferred from their filters.
    QString colorMode;

    [[nodiscard]] QJsonObject toJson() const;
    [[nodiscard]] static std::optional<FilterPreset> fromJson(const QJsonObject &root);
};

#endif
