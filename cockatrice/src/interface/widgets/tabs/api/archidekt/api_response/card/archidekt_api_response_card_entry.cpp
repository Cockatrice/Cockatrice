#include "archidekt_api_response_card_entry.h"

void ArchidektApiResponseCardEntry::fromJson(const QJsonObject &json)
{
    id = json.value("id").toInt();

    auto categoriesJson = json.value("categories").toArray();

    for (auto category : categoriesJson) {
        categories.append(category.toString());
    }

    companion = json.value("companion").toBool();
    flippedDefault = json.value("flippedDefault").toBool();
    label = json.value("label").toString();
    modifier = json.value("modifier").toString();
    quantity = json.value("quantity").toInt();
    customCmc = json.value("customCmc").toInt();
    // removedCategories = {""};
    createdAt = json.value("createdAt").toString();
    updatedAt = json.value("updatedAt").toString();
    deletedAt = json.value("deletedAt").toString();
    notes = json.value("notes").toString();
    card.fromJson(json.value("card").toObject());
}

void ArchidektApiResponseCardEntry::debugPrint() const
{
    qDebug() << "Id:" << id;
    qDebug() << "Categories:" << categories;
    qDebug() << "Companion:" << companion;
    qDebug() << "FlippedDefault:" << flippedDefault;
    qDebug() << "Label:" << label;
    qDebug() << "Modifier:" << modifier;
    qDebug() << "Quantity:" << quantity;
    qDebug() << "CustomCmc:" << customCmc;
    qDebug() << "RemovedCategories:" << removedCategories;
    qDebug() << "CreatedAt:" << createdAt;
    qDebug() << "UpdatedAt:" << updatedAt;
    qDebug() << "DeletedAt:" << deletedAt;
    qDebug() << "Notes:" << notes;
    card.debugPrint();
}