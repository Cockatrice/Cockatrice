#ifndef FEATURE_INTRO_ACTIONS_H
#define FEATURE_INTRO_ACTIONS_H

#include <QString>
#include <functional>

/**
 * @file feature_intro_actions.h
 * @brief The settings work behind a feature introduction, keyed by catalog id.
 *
 * The catalog stays pure data (see `feature_intro.h`) so it can compile into a
 * test target, and this is where the client-specific wiring lives: whether the
 * user already touched the feature, which behavior they have now, and how to
 * flip it either way. The service calls into this; nothing else should.
 */

/// Undoes an applied action; empty when there is nothing to undo.
using FeatureIntroAction = std::function<void()>;

struct FeatureIntroHandlers
{
    /// True when the user already changed this feature's settings themselves, so
    /// an introduction would only repeat what they did.
    std::function<bool()> isConfigured;
    /// True when the user is currently on the classic / off behavior.
    std::function<bool()> isAlternativeActive;
    /// Selects the new behavior; returns the undo.
    std::function<FeatureIntroAction()> applyAccept;
    /// Selects the classic / off behavior; returns the undo.
    std::function<FeatureIntroAction()> applyDecline;
};

/**
 * @brief Handlers for @a id; every member is empty when the id has no wiring.
 *
 * An entry without handlers still gets announced — it just cannot be flipped from
 * the banner, which is why every catalog entry shipping today has them.
 */
FeatureIntroHandlers featureIntroHandlers(const QString &id);

#endif // FEATURE_INTRO_ACTIONS_H
