import { renderHook } from '@testing-library/react';
import { useLocaleSort } from './useLocaleSort';

let mockLanguage = 'en';

vi.mock('react-i18next', () => ({
  useTranslation: () => ({
    i18n: {
      get language() {
        return mockLanguage;
      },
    },
  }),
}));

describe('useLocaleSort', () => {
  beforeEach(() => {
    mockLanguage = 'en';
  });

  test('sorts strings by locale using the valueGetter', () => {
    const arr = ['c', 'a', 'b'];
    const { result } = renderHook(() => useLocaleSort(arr, (v) => v));

    expect(result.current).toEqual(['a', 'b', 'c']);
  });

  test('sorts using valueGetter to resolve display values', () => {
    const lookup: Record<string, string> = { x: 'cherry', y: 'apple', z: 'banana' };
    const arr = ['x', 'y', 'z'];
    const { result } = renderHook(() => useLocaleSort(arr, (v) => lookup[v]));

    expect(result.current).toEqual(['y', 'z', 'x']);
  });

  test('handles empty array', () => {
    const { result } = renderHook(() => useLocaleSort([], (v) => v));

    expect(result.current).toEqual([]);
  });

  test('does not mutate the input array', () => {
    const arr = ['c', 'a', 'b'];
    renderHook(() => useLocaleSort(arr, (v) => v));

    expect(arr).toEqual(['c', 'a', 'b']);
  });

  test('updates when arr prop changes', () => {
    let arr = ['c', 'a'];
    const getter = (v: string) => v;
    const { result, rerender } = renderHook(() => useLocaleSort(arr, getter));

    expect(result.current).toEqual(['a', 'c']);

    arr = ['z', 'b', 'a'];
    rerender();

    expect(result.current).toEqual(['a', 'b', 'z']);
  });

  test('re-sorts when language changes', () => {
    // Swedish sorts ä after z; English sorts ä near a
    const arr = ['ä', 'b', 'z'];
    const getter = (v: string) => v;

    mockLanguage = 'en';
    const { result, rerender } = renderHook(() => useLocaleSort(arr, getter));
    const englishOrder = [...result.current];

    mockLanguage = 'sv';
    rerender();
    const swedishOrder = [...result.current];

    // In Swedish, ä comes after z
    expect(swedishOrder[swedishOrder.length - 1]).toBe('ä');
    // In English, ä sorts near 'a', before 'b'
    expect(englishOrder.indexOf('ä')).toBeLessThan(englishOrder.indexOf('b'));
  });
});
