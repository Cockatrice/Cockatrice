#include "feature_intro_actions.h"

#include "../settings/cache_settings.h"

#include <libcockatrice/settings/cards_display_settings.h>

namespace
{
constexpr auto kHoverCardPanelId = "cards-hover-panel";

bool cardsConfigured()
{
    const auto &cards = SettingsCache::instance().cardsDisplay();
    // Compare the values, not the presence of the keys. Every settings page writes
    // its widgets on accept, so a visit that changed nothing still stores both
    // triggers at the shipped default; treating presence as a deliberate choice
    // would then skip the announcement for anyone who opened the dialog, which is
    // the opposite of what this system is for. A value that differs from the
    // default is a choice already made, so there is nothing left to announce.
    return cards.getHoverZoomTrigger() != CardsDisplaySettings::HoverPanelTriggerHover ||
           cards.getHoverPanelTrigger() != CardsDisplaySettings::HoverPanelTriggerHover;
}

bool classicHoverActive()
{
    const auto &cards = SettingsCache::instance().cardsDisplay();
    return cards.getHoverZoomTrigger() == CardsDisplaySettings::HoverPanelTriggerOff &&
           cards.getHoverPanelTrigger() == CardsDisplaySettings::HoverPanelTriggerOff;
}

FeatureIntroHandlers hoverCardPanelHandlers()
{
    FeatureIntroHandlers handlers;
    handlers.isConfigured = cardsConfigured;
    handlers.isAlternativeActive = classicHoverActive;
    // The rework is already the shipped default, so keeping it normally needs no
    // write. After a decline it does: the triggers are Off, and the index's undo is
    // exactly this call, so it has to put both back. A no-op write would record
    // "you are using it" over settings that say otherwise.
    handlers.applyAccept = []() -> FeatureIntroAction {
        auto &cards = SettingsCache::instance().cardsDisplay();
        const int zoom = cards.getHoverZoomTrigger();
        const int panel = cards.getHoverPanelTrigger();
        if (zoom == CardsDisplaySettings::HoverPanelTriggerHover &&
            panel == CardsDisplaySettings::HoverPanelTriggerHover) {
            return FeatureIntroAction();
        }

        cards.setHoverZoomTrigger(CardsDisplaySettings::HoverPanelTriggerHover);
        cards.setHoverPanelTrigger(CardsDisplaySettings::HoverPanelTriggerHover);
        return [zoom, panel] {
            auto &restored = SettingsCache::instance().cardsDisplay();
            restored.setHoverZoomTrigger(zoom);
            restored.setHoverPanelTrigger(panel);
        };
    };
    handlers.applyDecline = [] {
        auto &cards = SettingsCache::instance().cardsDisplay();
        const int zoom = cards.getHoverZoomTrigger();
        const int panel = cards.getHoverPanelTrigger();
        cards.setHoverZoomTrigger(CardsDisplaySettings::HoverPanelTriggerOff);
        cards.setHoverPanelTrigger(CardsDisplaySettings::HoverPanelTriggerOff);
        return [zoom, panel] {
            auto &restored = SettingsCache::instance().cardsDisplay();
            restored.setHoverZoomTrigger(zoom);
            restored.setHoverPanelTrigger(panel);
        };
    };
    return handlers;
}
} // namespace

FeatureIntroHandlers featureIntroHandlers(const QString &id)
{
    if (id == QLatin1String(kHoverCardPanelId)) {
        return hoverCardPanelHandlers();
    }
    return {};
}
