import { renderHook } from '@testing-library/react';
import { useScryfallCard } from './useScryfallCard';

describe('useScryfallCard', () => {
  it('returns not-ready when card is null', () => {
    const { result } = renderHook(() => useScryfallCard(null));
    expect(result.current).toEqual({ smallUrl: null, normalUrl: null, ready: false });
  });

  it('returns not-ready when card has neither providerId nor name', () => {
    const { result } = renderHook(() => useScryfallCard({}));
    expect(result.current.ready).toBe(false);
    expect(result.current.smallUrl).toBeNull();
  });

  it('builds both small and normal URLs from providerId', () => {
    const { result } = renderHook(() =>
      useScryfallCard({ providerId: 'abc-123', name: 'Anything' }),
    );
    expect(result.current.ready).toBe(true);
    expect(result.current.smallUrl).toContain('/cards/abc-123');
    expect(result.current.smallUrl).toContain('version=small');
    expect(result.current.normalUrl).toContain('/cards/abc-123');
    expect(result.current.normalUrl).toContain('version=normal');
  });

  it('falls back to name when providerId is empty', () => {
    const { result } = renderHook(() =>
      useScryfallCard({ providerId: '', name: 'Lightning Bolt' }),
    );
    expect(result.current.ready).toBe(true);
    expect(result.current.smallUrl).toContain('/cards/named');
    expect(result.current.smallUrl).toContain('Lightning%20Bolt');
  });

  it('memoizes the result across re-renders with the same inputs', () => {
    const card = { providerId: 'abc-123' };
    const { result, rerender } = renderHook(({ c }) => useScryfallCard(c), {
      initialProps: { c: card },
    });
    const first = result.current;
    rerender({ c: card });
    expect(result.current).toBe(first);
  });
});
