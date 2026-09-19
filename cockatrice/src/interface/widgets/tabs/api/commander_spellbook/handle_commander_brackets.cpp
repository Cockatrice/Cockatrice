#include "handle_commander_brackets.h"

#include "../../../../../client/settings/cache_settings.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <libcockatrice/settings/commander_bracket_settings.h>

static const QUrl COMMANDER_BRACKET_JSON_URL(QStringLiteral("https://cockatrice.github.io/commander-brackets.json"));

HandleCommanderBrackets::HandleCommanderBrackets(QObject *parent)
    : QObject(parent), nam(new QNetworkAccessManager(this)), reply(nullptr)
{
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
    reply = qobject_cast<QNetworkReply *>(sender());

    if (reply->error() != QNetworkReply::NoError) {
        emit sigBracketDefinitionsDownloadFailed(reply->error());

        reply->deleteLater();
        return;
    }

    QJsonParseError parseError;

    auto document = QJsonDocument::fromJson(reply->readAll(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit sigBracketDefinitionsDownloadFailed(QNetworkReply::UnknownContentError);

        reply->deleteLater();
        return;
    }

    updateBracketDefinitions(document.toVariant().toMap());

    emit sigBracketDefinitionsDownloaded();

    reply->deleteLater();
}

void HandleCommanderBrackets::updateBracketDefinitions(const QVariantMap &jsonMap)
{
    const auto bracketList = jsonMap.value("brackets").toList();
    SettingsCache::instance().commanderBrackets().saveDefinitions(bracketList);
    SettingsCache::instance().commanderBrackets().reloadDefinitions(bracketList);
}
