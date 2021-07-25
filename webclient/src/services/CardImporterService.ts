// Fetch and parse card sets

class CardImporterService {
  importCards(url): Promise<any> {
    return fetch(url)
      .then(response => response.json())
      .then((json) => {
        const sortedSets = Object.keys(json.data)
          .map(key => json.data[key])
          .sort((a, b) => new Date(a.releaseDate).getTime() - new Date(b.releaseDate).getTime());

        const sets = sortedSets.map(({ cards, ...set}) => ({
          ...set,
          cards: cards.map(({ name }) => name),
        }));

        const unsortedCards = sortedSets.reduce((acc, set) => {
          set.cards.forEach(card => acc[card.name] = card);
          return acc;
        }, {});

        const cards = Object.keys(unsortedCards)
          .sort((a, b) => a.localeCompare(b))
          .map(key => unsortedCards[key]);

        return { cards, sets };
      });
  }

  importTokens(url): Promise<any> {
    return fetch(url)
      .then(response => response.text())
      .then((xmlString) => {
        const parser = new DOMParser();
        const dom = parser.parseFromString(xmlString, "application/xml");

        const tokens = Array.from(dom.querySelectorAll('card')).map(
          (tokenElement) => this.parseXmlAttributes(tokenElement)
        );

        return tokens;
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

      if (attributes[child.tagName]) {
        if (Array.isArray(attributes[child.tagName])) {
          attributes[child.tagName].push(parsedAttributes)
        } else {
          attributes[child.tagName] = [ parsedAttributes ];
        }
      } else {
        attributes[child.tagName] = parsedAttributes;
      }

      return attributes;
    }, {});
  }
}

export const cardImporterService = new CardImporterService();
