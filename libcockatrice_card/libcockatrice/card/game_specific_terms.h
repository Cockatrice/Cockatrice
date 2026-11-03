/**
 * @file game_specific_terms.h
 * @ingroup Cards
 */
//! \todo Document this file.

#ifndef GAME_SPECIFIC_TERMS_H
#define GAME_SPECIFIC_TERMS_H

#include <QCoreApplication>
#include <QString>

/*
 * Collection of traslatable property names used in games,
 * so we can use Game::Property instead of hardcoding strings.
 * Note: Mtg = "Maybe that game"
 */

// These remain only for reading older Cockatrice XML. New cards use Essence.
namespace EssenceProperties
{
inline const QString Cost = QStringLiteral("essencecost");
inline const QString Types = QStringLiteral("essence");
inline const QString Value = QStringLiteral("essencevalue");
}

// Continuum Universes CCG card-definition fields, not mutable match stats.
namespace ContinuumCardProperties
{
inline const QString Identity = QStringLiteral("identity");
inline const QString Alias = QStringLiteral("alias"); // Earlier exports may use this name.
inline const QString Attack = QStringLiteral("attack");
inline const QString Strength = QStringLiteral("strength"); // Legacy MSE field ID.
inline const QString Armour = QStringLiteral("armour");
inline const QString Armor = QStringLiteral("armor"); // Legacy American spelling.
inline const QString Health = QStringLiteral("health");
}

namespace Mtg
{
QString const CardType("type");
QString const ConvertedManaCost("cmc");
QString const Colors("colors");
QString const Loyalty("loyalty");
QString const MainCardType("maintype");
QString const ManaCost("manacost");
QString const PowTough("pt");
QString const Side("side");
QString const Layout("layout");
QString const ColorIdentity("coloridentity");

inline static const QString getNicePropertyName(QString key)
{
    if (key == EssenceProperties::Cost || key == ManaCost) {
        return QCoreApplication::translate("Essence", "Essence Cost");
    }
    if (key == EssenceProperties::Types || key == Colors || key == ColorIdentity) {
        return QCoreApplication::translate("Essence", "Essence");
    }
    if (key == EssenceProperties::Value || key == ConvertedManaCost) {
        return QCoreApplication::translate("Essence", "Essence Value");
    }
    if (key == ContinuumCardProperties::Identity || key == ContinuumCardProperties::Alias) {
        return QCoreApplication::translate("Continuum", "Identity");
    }
    if (key == ContinuumCardProperties::Attack || key == ContinuumCardProperties::Strength) {
        return QCoreApplication::translate("Continuum", "Attack");
    }
    if (key == ContinuumCardProperties::Armour || key == ContinuumCardProperties::Armor) {
        return QCoreApplication::translate("Continuum", "Armour");
    }
    if (key == ContinuumCardProperties::Health) {
        return QCoreApplication::translate("Continuum", "Health");
    }
    if (key == CardType) {
        return QCoreApplication::translate("Mtg", "Card Type");
    }
    if (key == Loyalty) {
        return QCoreApplication::translate("Mtg", "Loyalty");
    }
    if (key == MainCardType) {
        return QCoreApplication::translate("Mtg", "Main Card Type");
    }
    if (key == PowTough) {
        return QCoreApplication::translate("Continuum", "ATK/HP");
    }
    if (key == Side) {
        return QCoreApplication::translate("Mtg", "Side");
    }
    if (key == Layout) {
        return QCoreApplication::translate("Mtg", "Layout");
    }
    return key;
}
} // namespace Mtg

#endif
