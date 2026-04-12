import { guid } from './guid.util';

describe('guid', () => {
  it('returns a string', () => {
    expect(typeof guid()).toBe('string');
  });

  it('matches UUID v4 pattern', () => {
    const uuidPattern = /^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$/;
    expect(guid()).toMatch(uuidPattern);
  });

  it('returns deterministic value when Math.random is mocked', () => {
    const spy = vi.spyOn(Math, 'random').mockReturnValue(0.5);
    const result = guid();
    expect(result).toBe(guid());
    spy.mockRestore();
  });
});
