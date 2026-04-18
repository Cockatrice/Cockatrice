import { cardImporterService } from './CardImporterService';

const mockFetch = vi.fn();
globalThis.fetch = mockFetch;

function jsonResponse(body: unknown, contentType = 'application/json') {
  return {
    ok: true,
    headers: new Headers({ 'Content-Type': contentType }),
    json: () => Promise.resolve(body),
  } as unknown as Response;
}

function textResponse(body: string, ok = true) {
  return {
    ok,
    headers: new Headers({ 'Content-Type': 'application/xml' }),
    text: () => Promise.resolve(body),
  } as unknown as Response;
}

function failedResponse(status = 500) {
  return {
    ok: false,
    status,
    headers: new Headers({ 'Content-Type': 'application/json' }),
    json: () => Promise.resolve({}),
    text: () => Promise.resolve(''),
  } as unknown as Response;
}

// Minimal MTGJSON-shaped fixture
const mtgjsonFixture = {
  data: {
    SET_B: {
      code: 'SET_B',
      name: 'Set B',
      releaseDate: '2020-06-01',
      cards: [
        { name: 'Zebra' },
        { name: 'Alpha' },
      ],
      tokens: [{ name: 'Token B' }],
    },
    SET_A: {
      code: 'SET_A',
      name: 'Set A',
      releaseDate: '2019-01-01',
      cards: [
        { name: 'Alpha' },
        { name: 'Beta' },
      ],
      tokens: [{ name: 'Token A' }],
    },
  },
};

describe('CardImporterService', () => {
  describe('importCards', () => {
    it('fetches and parses valid MTGJSON data into sorted cards and sets', async () => {
      mockFetch.mockResolvedValue(jsonResponse(mtgjsonFixture));

      const { cards, sets } = await cardImporterService.importCards('http://example.com/cards.json');

      expect(cards).toHaveLength(3);
      expect(cards[0].name).toBe('Alpha');
      expect(cards[1].name).toBe('Beta');
      expect(cards[2].name).toBe('Zebra');

      expect(sets).toHaveLength(2);
      expect(sets[0].name).toBe('Set A');
      expect(sets[1].name).toBe('Set B');
    });

    it('sorts sets by releaseDate ascending', async () => {
      mockFetch.mockResolvedValue(jsonResponse(mtgjsonFixture));

      const { sets } = await cardImporterService.importCards('http://example.com/cards.json');

      expect(sets[0].code).toBe('SET_A');
      expect(sets[1].code).toBe('SET_B');
    });

    it('deduplicates cards by name, keeping last occurrence (later set wins)', async () => {
      mockFetch.mockResolvedValue(jsonResponse(mtgjsonFixture));

      const { cards } = await cardImporterService.importCards('http://example.com/cards.json');

      // Alpha appears in both SET_A and SET_B; SET_B is later so its version overwrites
      // 3 unique names: Alpha (deduped), Beta (SET_A only), Zebra (SET_B only)
      expect(cards).toHaveLength(3);
      expect(cards.map(c => c.name)).toEqual(['Alpha', 'Beta', 'Zebra']);
    });

    it('maps set cards and tokens to name arrays', async () => {
      mockFetch.mockResolvedValue(jsonResponse(mtgjsonFixture));

      const { sets } = await cardImporterService.importCards('http://example.com/cards.json');

      expect(sets[0].cards).toEqual(['Alpha', 'Beta']);
      expect(sets[0].tokens).toEqual(['Token A']);
    });

    it('rejects when response is not ok', async () => {
      mockFetch.mockResolvedValue(failedResponse(404));

      await expect(cardImporterService.importCards('http://example.com/cards.json'))
        .rejects.toThrow('Card import must be in valid MTG JSON format');
    });

    it('rejects when Content-Type does not contain application/json', async () => {
      mockFetch.mockResolvedValue({
        ok: true,
        headers: new Headers({ 'Content-Type': 'text/html' }),
        json: () => Promise.resolve({}),
      } as unknown as Response);

      await expect(cardImporterService.importCards('http://example.com/cards.json'))
        .rejects.toThrow('Card import must be in valid MTG JSON format');
    });

    it('accepts Content-Type with charset parameter', async () => {
      mockFetch.mockResolvedValue(jsonResponse(mtgjsonFixture, 'application/json; charset=utf-8'));

      const { cards } = await cardImporterService.importCards('http://example.com/cards.json');
      expect(cards.length).toBeGreaterThan(0);
    });

    it('rejects when JSON structure is invalid (missing data key)', async () => {
      mockFetch.mockResolvedValue(jsonResponse({ notData: {} }));

      await expect(cardImporterService.importCards('http://example.com/cards.json'))
        .rejects.toThrow('Card import must be in valid MTG JSON format');
    });

    it('preserves the original error as cause', async () => {
      mockFetch.mockResolvedValue(jsonResponse({ notData: {} }));

      try {
        await cardImporterService.importCards('http://example.com/cards.json');
        expect.fail('should have thrown');
      } catch (err) {
        expect((err as Error).cause).toBeDefined();
      }
    });
  });

  describe('importTokens', () => {
    const validXml = `<?xml version="1.0" encoding="UTF-8"?>
<cockatrice_tokens>
  <card>
    <name value="Soldier" />
    <set value="M21" picURL="http://example.com/soldier.png" />
    <tablerow value="1" />
  </card>
</cockatrice_tokens>`;

    it('fetches and parses valid XML into token objects', async () => {
      mockFetch.mockResolvedValue(textResponse(validXml));

      const tokens = await cardImporterService.importTokens('http://example.com/tokens.xml');

      expect(tokens).toHaveLength(1);
      expect(tokens[0]).toHaveProperty('name');
    });

    it('parses token attributes correctly', async () => {
      mockFetch.mockResolvedValue(textResponse(validXml));

      const tokens = await cardImporterService.importTokens('http://example.com/tokens.xml');

      const token = tokens[0] as Record<string, any>;
      expect(token.name.value).toBe('Soldier');
      expect(token.set.value).toBe('M21');
      expect(token.set.picURL).toBe('http://example.com/soldier.png');
    });

    it('rejects when response is not ok', async () => {
      mockFetch.mockResolvedValue({ ok: false, text: () => Promise.resolve('') } as unknown as Response);

      await expect(cardImporterService.importTokens('http://example.com/tokens.xml'))
        .rejects.toThrow('Failed to fetch');
    });

    it('rejects when XML is malformed', async () => {
      mockFetch.mockResolvedValue(textResponse('<not-valid-xml>'));

      await expect(cardImporterService.importTokens('http://example.com/tokens.xml'))
        .rejects.toThrow('Token import must be in valid MTG XML format');
    });

    it('returns empty array when XML has no card elements', async () => {
      const emptyXml = '<?xml version="1.0"?><cockatrice_tokens></cockatrice_tokens>';
      mockFetch.mockResolvedValue(textResponse(emptyXml));

      const tokens = await cardImporterService.importTokens('http://example.com/tokens.xml');
      expect(tokens).toEqual([]);
    });

    it('preserves the original error as cause on parse failure', async () => {
      mockFetch.mockResolvedValue(textResponse('<not-valid-xml>'));

      try {
        await cardImporterService.importTokens('http://example.com/tokens.xml');
        expect.fail('should have thrown');
      } catch (err) {
        expect((err as Error).cause).toBeDefined();
      }
    });
  });

  describe('parseXmlAttributes', () => {
    function parseXml(xml: string) {
      const dom = new DOMParser().parseFromString(xml, 'application/xml');
      return (cardImporterService as any).parseXmlAttributes(dom.documentElement);
    }

    it('parses simple child elements into key-value pairs', () => {
      const result = parseXml('<card><name value="Soldier" /></card>');
      expect(result.name.value).toBe('Soldier');
    });

    it('parses nested elements recursively', () => {
      const result = parseXml('<card><prop><cmc value="2" /></prop></card>');
      expect(result.prop.value).toHaveProperty('cmc');
      expect(result.prop.value.cmc.value).toBe('2');
    });

    it('includes XML attributes alongside value', () => {
      const result = parseXml('<card><set value="M21" picURL="http://img.png" /></card>');
      expect(result.set.value).toBe('M21');
      expect(result.set.picURL).toBe('http://img.png');
    });

    it('converts duplicate tag names into an array preserving all values', () => {
      const result = parseXml(
        '<card><related value="Token A" /><related value="Token B" /></card>'
      );
      expect(Array.isArray(result.related)).toBe(true);
      expect(result.related).toHaveLength(2);
      expect(result.related[0].value).toBe('Token A');
      expect(result.related[1].value).toBe('Token B');
    });

    it('appends to existing array for 3+ duplicate tag names', () => {
      const result = parseXml(
        '<card><set value="A" /><set value="B" /><set value="C" /></card>'
      );
      expect(Array.isArray(result.set)).toBe(true);
      expect(result.set).toHaveLength(3);
      expect(result.set[0].value).toBe('A');
      expect(result.set[1].value).toBe('B');
      expect(result.set[2].value).toBe('C');
    });

    it('reads innerHTML as value for leaf elements without children', () => {
      const result = parseXml('<card><text>Some card text</text></card>');
      expect(result.text.value).toBe('Some card text');
    });
  });
});
