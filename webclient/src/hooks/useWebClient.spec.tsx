import { renderHook } from '@testing-library/react';
import { ReactNode } from 'react';
import { WebClientProvider, useWebClient } from './useWebClient';

vi.mock('@app/websocket', () => ({
  WebClient: class MockWebClient {},
}));

vi.mock('@app/api', () => ({
  createWebClientRequest: vi.fn(() => 'request'),
  createWebClientResponse: vi.fn(() => 'response'),
}));

function Wrapper({ children }: { children: ReactNode }) {
  return <WebClientProvider>{children}</WebClientProvider>;
}

describe('useWebClient', () => {
  test('provides the WebClient instance to children', () => {
    const { result } = renderHook(() => useWebClient(), { wrapper: Wrapper });

    expect(result.current).toBeDefined();
    expect(result.current.constructor.name).toBe('MockWebClient');
  });

  test('throws when used outside WebClientProvider', () => {
    // Suppress React error boundary console output
    const spy = vi.spyOn(console, 'error').mockImplementation(() => {});

    expect(() => {
      renderHook(() => useWebClient());
    }).toThrow('useWebClient must be used within a WebClientProvider');

    spy.mockRestore();
  });

  test('returns the same instance across re-renders', () => {
    const { result, rerender } = renderHook(() => useWebClient(), {
      wrapper: Wrapper,
    });

    const first = result.current;
    rerender();
    const second = result.current;

    expect(first).toBe(second);
  });
});
