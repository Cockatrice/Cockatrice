// Fetch and parse card sets

import { App } from '@app/types';

class CardImporterService {
  importCards(url: string): Promise<{ cards: App.Card[]; sets: App.Set[] }> {
    const error = 'Card import must be in valid MTG JSON format';

    return fetch(url)
      .then(response => {
        if (!response.ok) {
          throw new Error(error);
        }

        const contentType = response.headers.get('Content-Type') ?? '';
        if (!contentType.includes('application/json')) {
          throw new Error(error);
        }

        return response.json();
      })
      .then((json) => {
        try {
          const sortedSets = Object.keys(json.data)
            .map(key => json.data[key])
            .sort((a, b) => new Date(a.releaseDate).getTime() - new Date(b.releaseDate).getTime());

          const sets = sortedSets.map(({ cards, tokens, ...set }) => ({
            ...set,
            cards: cards.map(({ name }) => name),
            tokens: tokens.map(({ name }) => name),
          }));

          const unsortedCards = sortedSets.reduce((acc, set) => {
            set.cards.forEach(card => acc[card.name] = card);
            return acc;
          }, {});

          const cards = Object.keys(unsortedCards)
            .sort((a, b) => a.localeCompare(b))
            .map(key => unsortedCards[key]);

          return { cards, sets };
        } catch (err) {
          throw new Error(error, { cause: err });
        }
      });
  }

  importTokens(url: string): Promise<Record<string, unknown>[]> {
    const error = 'Token import must be in valid MTG XML format';

    return fetch(url)
      .then(response => {
        if (!response.ok) {
          throw new Error('Failed to fetch');
        }

        return response.text()
      })
      .then((xmlString) => {
        try {
          const parser = new DOMParser();
          const dom = parser.parseFromString(xmlString, 'application/xml');

          if (dom.querySelector('parsererror')) {
            throw new Error(error);
          }

          const tokens = Array.from(dom.querySelectorAll('card')).map(
            (tokenElement) => this.parseXmlAttributes(tokenElement)
          );

          return tokens;
        } catch (err) {
          throw new Error(error, { cause: err });
        }
      })
  }

  private parseXmlAttributes(dom: Element) {
    return Array.from(dom.children).reduce((attributes, child) => {
      const value = child.children.length ? this.parseXmlAttributes(child) : child.innerHTML;

      let parsedAttributes = { value };

      if (child.attributes.length) {
        const childAttributes = Array.from(child.attributes).reduce((acc, { name, value }) => {
          acc[name] = value;
          return acc;
        }, {});

        parsedAttributes = {
          ...parsedAttributes,
          ...childAttributes,
        };
      }

      // @TODO: clean this up and normalize what i'm returning
      if (attributes[child.tagName]) {
        if (Array.isArray(attributes[child.tagName])) {
          attributes[child.tagName].push(parsedAttributes)
        } else {
          attributes[child.tagName] = [attributes[child.tagName], parsedAttributes];
        }
      } else {
        attributes[child.tagName] = parsedAttributes;
      }

      return attributes;
    }, {});
  }
}

export const cardImporterService = new CardImporterService();
