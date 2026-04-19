import { setPendingOptions, consumePendingOptions } from './connectionState';
import type { WebSocketConnectOptions } from '../types/ConnectOptions';

const opts = (over: Partial<WebSocketConnectOptions> = {}): WebSocketConnectOptions => ({
  type: 'login',
  host: 'h',
  port: '1',
  userName: 'u',
  ...over,
} as unknown as WebSocketConnectOptions);

describe('connectionState', () => {
  beforeEach(() => {
    // Drain any value lingering from prior tests.
    consumePendingOptions();
  });

  it('returns null when nothing has been set', () => {
    expect(consumePendingOptions()).toBeNull();
  });

  it('round-trips a value through set → consume', () => {
    const value = opts({ host: 'a' });
    setPendingOptions(value);
    expect(consumePendingOptions()).toBe(value);
  });

  it('consume is one-shot — second call returns null', () => {
    setPendingOptions(opts());
    expect(consumePendingOptions()).not.toBeNull();
    expect(consumePendingOptions()).toBeNull();
  });

  it('a second set replaces the prior value (no queue semantics)', () => {
    const first = opts({ host: 'a' });
    const second = opts({ host: 'b' });
    setPendingOptions(first);
    setPendingOptions(second);
    expect(consumePendingOptions()).toBe(second);
    expect(consumePendingOptions()).toBeNull();
  });
});
