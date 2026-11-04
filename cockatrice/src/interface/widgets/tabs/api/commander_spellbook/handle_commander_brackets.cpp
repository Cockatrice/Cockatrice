#include "handle_commander_brackets.h"

#include "../../../../../client/settings/cache_settings.h"

#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QSet>
#include <QString>
#include <QStringLiteral>
#include <QUrl>
#include <QVariant>
#include <libcockatrice/network/network_manager.h>
#include <libcockatrice/settings/commander_bracket_settings.h>

static const QUrl COMMANDER_BRACKET_JSON_URL(QStringLiteral("https://cockatrice.github.io/commander-brackets.json"));

HandleCommanderBrackets::HandleCommanderBrackets(QObject *parent)
    : QObject(parent), nam(NetworkManagerFactory::create(this)), reply(nullptr)
{
}

HandleCommanderBrackets::~HandleCommanderBrackets()
{
    if (reply) {
        disconnect(reply, nullptr, this, nullptr);
        reply->abort();
    }
}

void HandleCommanderBrackets::downloadBracketDefinitions()
{
    if (reply) {
        return;
    }

    reply = nam->get(QNetworkRequest(COMMANDER_BRACKET_JSON_URL));

    connect(reply, &QNetworkReply::finished, this, &HandleCommanderBrackets::actFinishParsingDownloadedData);
}

void HandleCommanderBrackets::actFinishParsingDownloadedData()
{
    auto *finishedReply = qobject_cast<QNetworkReply *>(sender());
    if (!finishedReply || finishedReply != reply) {
        return;
    }

    // Clear the active request before notifying listeners, which may immediately retry.
    reply = nullptr;
    finishedReply->deleteLater();

    if (finishedReply->error() != QNetworkReply::NoError) {
        emit sigBracketDefinitionsDownloadFailed(finishedReply->error());
        return;
    }

    QJsonParseError parseError;

    auto document = QJsonDocument::fromJson(finishedReply->readAll(), &parseError);

    if (parseError.error != QJsonParseError::NoError || !document.isObject() ||
        !document.object().value("brackets").isArray() || document.object().value("brackets").toArray().isEmpty()) {
        emit sigBracketDefinitionsDownloadFailed(QNetworkReply::UnknownContentError);
        return;
    }

    // Validate the complete replacement before touching the last usable definitions.
    // Tags are QSettings group names, so path separators must not create nested groups.
    QSet<QString> tags;
    for (const auto &value : document.object().value("brackets").toArray()) {
        const auto definition = value.toObject();
        const auto tag = definition.value("tag").toString();
        if (!value.isObject() || tag.trimmed().isEmpty() || tag.contains('/') || tag.contains('\\') ||
            tags.contains(tag) || definition.value("officialName").toString().trimmed().isEmpty() ||
            definition.value("displayName").toString().trimmed().isEmpty() ||
            !definition.value("explanation").isString()) {
            emit sigBracketDefinitionsDownloadFailed(QNetworkReply::UnknownContentError);
            return;
        }
        tags.insert(tag);
    }

    updateBracketDefinitions(document.toVariant().toMap());

    emit sigBracketDefinitionsDownloaded();
}

void HandleCommanderBrackets::updateBracketDefinitions(const QVariantMap &jsonMap)
{
    const auto bracketList = jsonMap.value("brackets").toList();
    SettingsCache::instance().commanderBrackets().saveDefinitions(bracketList);
    SettingsCache::instance().commanderBrackets().reloadDefinitions(bracketList);
}
