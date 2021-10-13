import {
  URL_REGEX,
  MESSAGE_SENDER_REGEX,
  MENTION_REGEX,
  CARD_CALLOUT_REGEX,
  CALLOUT_BOUNDARY_REGEX,
} from './constants';

describe('RegEx', () => {
  describe('URL_REGEX', () => {
    it('should match and capture whole url in main capture group', () => {
      const test = [
        'http://example.com',
        'https://example.com',
        'https://www.example.com',
      ];

      test.forEach(str => {
        const match = str.match(URL_REGEX);

        expect(match).toBeDefined();
        expect(match[0]).toBe(str);
      });
    });

    it('should not match bad urls', () => {
      const test = [
        'htt://example.com',
        'https:/example.com',
        'https//www.example.com',
        'www.example.com',
        'example.com',
      ];

      test.forEach(str =>
        expect(str.match(URL_REGEX)).toBe(null)
      );
    });
  });

  describe('MESSAGE_SENDER_REGEX', () => {
    it('should match and capture sender name in second capture group', () => {
      const sender = 'sender';
      const match = `${sender}: message`.match(MESSAGE_SENDER_REGEX);

      expect(match).toBeDefined();
      expect(match[1]).toBe(sender);
    });

    it('should not match if spaces before :', () => {
      const test = [
        ' sender: message',
        'sender : message',
        ' sender : message',
      ];

      test.forEach(str =>
        expect(str.match(URL_REGEX)).toBe(null)
      );
    });
  });

  describe('MENTION_REGEX', () => {
    it('should match and capture user mentions in second capture group', () => {
      expect('@mention'.match(MENTION_REGEX)[0]).toBe('@mention');
      expect('@mention '.match(MENTION_REGEX)[0]).toBe('@mention');
      expect(' @mention'.match(MENTION_REGEX)[0]).toBe(' @mention');
      expect(' @mention '.match(MENTION_REGEX)[0]).toBe(' @mention');
      expect('leading @mention'.match(MENTION_REGEX)[0]).toBe(' @mention');
      expect('leading @mention trailing'.match(MENTION_REGEX)[0]).toBe(' @mention');
      expect('@mention trailing'.match(MENTION_REGEX)[0]).toBe('@mention');
    });

    it('should not match preceded by character', () => {
      const test = [
        'leading@mention',
      ];

      test.forEach(str =>
        expect(str.match(MENTION_REGEX)).toBe(null)
      );
    });
  });
});
