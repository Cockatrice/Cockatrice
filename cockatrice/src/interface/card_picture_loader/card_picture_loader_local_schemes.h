#ifndef COCKATRICE_CARD_PICTURE_LOADER_LOCAL_SCHEMES_H
#define COCKATRICE_CARD_PICTURE_LOADER_LOCAL_SCHEMES_H

#include <QList>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

namespace CardPictureLoaderLocalSchemes
{

enum class NamingScheme
{
    NameOnly,
    Name_Set,
    Name_Set_Collector,
    Set_Collector_Name,
    Name_ProviderId,
    Set_Folder_Name_ProviderId,
    Set_Folder_Name_Set_Collector
};

struct NamingSchemeInfo
{
    NamingScheme id;
    QString displayName;
    QString pattern;
};

inline const QList<NamingSchemeInfo> &importSchemes()
{
    static QList<NamingSchemeInfo> list = {
        {NamingScheme::Name_ProviderId, "Card Name + Provider ID", "{name}_{providerId}"},
        {NamingScheme::Name_Set_Collector, "Card Name + Set + Collector", "{name}_{set}_{collector}"},
        {NamingScheme::Set_Collector_Name, "Set + Collector + Card Name", "{set}_{collector}_{name}"},
        {NamingScheme::Name_Set, "Card Name + Set", "{name}_{set}"},
        {NamingScheme::NameOnly, "Card Name", "{name}"},
    };
    return list;
}

inline const QList<NamingSchemeInfo> &exportSchemes()
{
    static QList<NamingSchemeInfo> list = {
        {NamingScheme::Set_Folder_Name_ProviderId, "Set Folder / Name + Provider ID", "{set}/{name}_{providerId}"},
        {NamingScheme::Set_Folder_Name_Set_Collector, "Set Folder / Name + Set Name + Collector",
         "{set}/{name}_{set}_{collector}"},
        {NamingScheme::Name_ProviderId, "Card Name + Provider ID", "{name}_{providerId}"},
        {NamingScheme::Name_Set_Collector, "Card Name + Set + Collector", "{name}_{set}_{collector}"},
        {NamingScheme::Set_Collector_Name, "Set + Collector + Card Name", "{set}_{collector}_{name}"},
    };
    return list;
}

inline QString expandPattern(const QString &pattern,
                             const QString &name,
                             const QString &set,
                             const QString &collector,
                             const QString &providerId)
{
    QString result = pattern;

    auto replaceIfPresent = [&](const QString &token, const QString &value) -> bool {
        if (!result.contains(token))
            return true;

        if (value.isEmpty())
            return false;

        result.replace(token, value);
        return true;
    };

    if (!replaceIfPresent("{name}", name))
        return {};
    if (!replaceIfPresent("{set}", set))
        return {};
    if (!replaceIfPresent("{collector}", collector))
        return {};
    if (!replaceIfPresent("{providerId}", providerId))
        return {};

    return result;
}

inline QStringList
generateImportVariants(const QString &name, const QString &set, const QString &collector, const QString &providerId)
{
    QStringList variants;
    const QStringList separators = {"_", "-"};

    for (const auto &scheme : importSchemes()) {
        for (const QString &sep : separators) {

            QString pattern = scheme.pattern;
            pattern.replace("_", sep);

            QString v = expandPattern(pattern, name, set, collector, providerId);
            if (!v.isEmpty())
                variants << v;
        }
    }

    return variants;
}

} // namespace CardPictureLoaderLocalSchemes

#endif // COCKATRICE_CARD_PICTURE_LOADER_LOCAL_SCHEMES_H