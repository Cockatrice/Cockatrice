#include "archidekt_api_response_card_entry.h"

void ArchidektApiResponseCardEntry::fromJson(const QJsonObject &json)
{
    id = json.value("id").toInt();

    categories.clear();

    auto categoriesJson = json.value("categories").toArray();

    for (const auto &categoryValue : categoriesJson) {
        Category cat;

        if (categoryValue.isObject()) {
            QJsonObject obj = categoryValue.toObject();

            cat.id = obj.value("id").toInt();
            cat.name = obj.value("name").toString();
            cat.isPremier = obj.value("isPremier").toBool();
            cat.includedInDeck = obj.value("includedInDeck").toBool();
            cat.includedInPrice = obj.value("includedInPrice").toBool();
        } else if (categoryValue.isString()) {
            cat.name = categoryValue.toString();

            // assume mainboard unless known otherwise
            cat.includedInDeck = true;
        }

        categories.append(cat);
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
    for (auto category : categories) {
        qDebug() << "Category ID:" << category.id;
        qDebug() << "Category Name:" << category.name;
        qDebug() << "Category Premier:" << category.isPremier;
        qDebug() << "Category Included in Deck:" << category.includedInDeck;
        qDebug() << "Category Included in Price:" << category.includedInPrice;
    }
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