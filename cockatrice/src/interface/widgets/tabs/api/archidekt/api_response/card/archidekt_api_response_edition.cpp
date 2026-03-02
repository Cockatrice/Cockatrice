#include "archidekt_api_response_edition.h"

void ArchidektApiResponseEdition::fromJson(const QJsonObject &json)
{
    editionCode = json.value("editioncode").toString();
    editionName = json.value("editionname").toString();
    editionDate = json.value("editiondate").toString();
    editionType = json.value("editiontype").toString();
    mtgoCode = json.value("mtgocode").toString();
}

void ArchidektApiResponseEdition::debugPrint() const
{
    qDebug() << "Edition Code: " << editionCode;
    qDebug() << "Edition Name: " << editionName;
    qDebug() << "Edition Date: " << editionDate;
    qDebug() << "Edition Type: " << editionType;
    qDebug() << "Mtgo Code: " << mtgoCode;
}