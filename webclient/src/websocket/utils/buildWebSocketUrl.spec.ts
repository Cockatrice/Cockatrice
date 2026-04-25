import { describe, it, expect } from 'vitest';

import { buildWebSocketUrl } from './buildWebSocketUrl';

describe('buildWebSocketUrl', () => {
  it('includes the port for direct (pathless) endpoints', () => {
    expect(buildWebSocketUrl('wss', 'mtg.chickatrice.net', '443')).toBe(
      'wss://mtg.chickatrice.net:443',
    );
  });

  it('drops the port for nginx-proxied hosts that bake a path into the host string', () => {
    expect(buildWebSocketUrl('wss', 'server.cockatrice.us/servatrice', '4748')).toBe(
      'wss://server.cockatrice.us/servatrice',
    );
  });

  it('preserves multi-segment proxy paths', () => {
    expect(buildWebSocketUrl('wss', 'example.com/foo/bar', '443')).toBe(
      'wss://example.com/foo/bar',
    );
  });

  it('accepts a numeric port for direct endpoints', () => {
    expect(buildWebSocketUrl('ws', 'localhost', 4748)).toBe('ws://localhost:4748');
  });

  it('honors the ws protocol even for nginx-proxied hosts', () => {
    expect(buildWebSocketUrl('ws', 'localhost/servatrice', 4748)).toBe(
      'ws://localhost/servatrice',
    );
  });
});
