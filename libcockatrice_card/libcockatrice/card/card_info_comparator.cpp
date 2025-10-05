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
    // Determine the type of QVariant based on Qt version
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (a.typeId() != b.typeId()) {
#else
    if (a.type() != b.type()) {
#endif
        // If they are not the same type, compare as strings
        return a.toString() < b.toString();
    }

    // Perform type-specific comparison
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    switch (static_cast<int>(a.typeId())) {
#else
    switch (static_cast<int>(a.type())) {
#endif
        case static_cast<int>(QMetaType::Int):
            return a.toInt() < b.toInt();
        case static_cast<int>(QMetaType::Double):
            return a.toDouble() < b.toDouble();
        case static_cast<int>(QMetaType::QString):
            return a.toString() < b.toString();
        case static_cast<int>(QMetaType::Bool):
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
