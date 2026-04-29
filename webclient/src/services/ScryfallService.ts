import { App } from '@app/types';

const SCRYFALL_API = 'https://api.scryfall.com';

/**
 * Build the `/cards/{id}` image URL for a known Scryfall card ID.
 *
 * Not called by runtime — `useScryfallCard` goes through
 * {@link getScryfallUrl} below. Retained because the spec pins the URL
 * shape independently, which has caught Scryfall API-version mismatches.
 */
export function getScryfallUrlById(
  providerId: string,
  size: App.ScryfallImageSize = App.ScryfallImageSize.Small,
): string {
  const id = encodeURIComponent(providerId);
  return `${SCRYFALL_API}/cards/${id}?format=image&version=${size}`;
}

/**
 * Build the `/cards/named?exact=` image URL for a card-name lookup.
 * See {@link getScryfallUrlById} for why this stays exported.
 */
export function getScryfallUrlByName(
  name: string,
  size: App.ScryfallImageSize = App.ScryfallImageSize.Small,
): string {
  const exact = encodeURIComponent(name);
  return `${SCRYFALL_API}/cards/named?exact=${exact}&format=image&version=${size}`;
}

/**
 * Runtime dispatcher: prefers `providerId` (Scryfall ID), falls back to
 * card name, returns null when neither is available. This is the only
 * function the UI layer calls.
 */
export function getScryfallUrl(
  card: { providerId?: string; name?: string },
  size: App.ScryfallImageSize = App.ScryfallImageSize.Small,
): string | null {
  if (card.providerId) {
    return getScryfallUrlById(card.providerId, size);
  }
  if (card.name) {
    return getScryfallUrlByName(card.name, size);
  }
  return null;
}
