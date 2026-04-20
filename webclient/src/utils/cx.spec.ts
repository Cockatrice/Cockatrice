import { cx } from './cx';

describe('cx', () => {
  it('joins strings with spaces', () => {
    expect(cx('a', 'b', 'c')).toBe('a b c');
  });

  it('drops falsy values', () => {
    expect(cx('a', false, null, undefined, 0, 'b')).toBe('a b');
  });

  it('applies object toggles', () => {
    expect(cx('a', { b: true, c: false, d: 1 })).toBe('a b d');
  });

  it('flattens nested arrays', () => {
    expect(cx('a', ['b', ['c', ['d']]])).toBe('a b c d');
  });

  it('returns an empty string for nothing', () => {
    expect(cx()).toBe('');
    expect(cx(false, null, undefined)).toBe('');
  });

  it('mixes strings, objects, and arrays', () => {
    const tapped = true as boolean;
    expect(
      cx('card-slot', tapped && 'card-slot--tapped', {
        'card-slot--face-down': true,
        'card-slot--dragging': false,
      }),
    ).toBe('card-slot card-slot--tapped card-slot--face-down');
  });
});
