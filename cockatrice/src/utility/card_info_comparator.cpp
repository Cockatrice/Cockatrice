#include "card_info_comparator.h"

CardInfoComparator::CardInfoComparator(const QStringList &properties, Qt::SortOrder order)
    : m_properties(properties), m_order(order)
{
}

bool CardInfoComparator::operator()(const CardInfoPtr &a, const CardInfoPtr &b) const
{
    // Iterate over each property in the list
    for (const QString &property : m_properties) {
        QVariant valueA = getProperty(a, property);
        QVariant valueB = getProperty(b, property);

        // Compare the current property
        if (valueA != valueB) {
            // If values differ, perform comparison
            return compareVariants(valueA, valueB) ? (m_order == Qt::AscendingOrder) : (m_order == Qt::DescendingOrder);
        }
    }

    // If all properties are equal, return false (indicating they are considered equal for sorting purposes)
    return false;
}

bool CardInfoComparator::compareVariants(const QVariant &a, const QVariant &b) const
{
    if (a.typeId() != b.typeId()) {
        // If they are not the same type, compare as strings
        return a.toString() < b.toString();
    }

    // Perform type-specific comparison
    switch (a.typeId()) {
        case QVariant::Int:
            return a.toInt() < b.toInt();
        case QVariant::Double:
            return a.toDouble() < b.toDouble();
        case QVariant::String:
            return a.toString() < b.toString();
        case QVariant::Bool:
            return a.toBool() < b.toBool();
        default:
            // Default to comparing as strings
            return a.toString() < b.toString();
    }
}

QVariant CardInfoComparator::getProperty(const CardInfoPtr &card, const QString &property) const
{
    // Check if the property exists in the main fields of the class
    if (property == "name") {
        return card->getName();
    } else if (property == "text") {
        return card->getText();
    } else if (property == "isToken") {
        return card->getIsToken();
    }

    // Otherwise, check if it's a custom property in the QVariantHash
    if (card->hasProperty(property)) {
        return card->getProperty(property);
    }

    return QVariant(); // Return an invalid variant if the property does not exist
}
