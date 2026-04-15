vi.mock('../../generated/proto/event_server_identification_pb', () => ({
  Event_ServerIdentification_ServerOptions: { SupportsPasswordHash: 2 },
}));

import { hashPassword, generateSalt, passwordSaltSupported } from './passwordHasher';

describe('hashPassword', () => {
  it('returns a string starting with the salt', () => {
    const result = hashPassword('mysalt', 'mypassword');
    expect(result.startsWith('mysalt')).toBe(true);
  });

  it('returns the same value for the same inputs (deterministic)', () => {
    expect(hashPassword('salt', 'pass')).toBe(hashPassword('salt', 'pass'));
  });

  it('returns different values for different salts', () => {
    expect(hashPassword('salt1', 'pass')).not.toBe(hashPassword('salt2', 'pass'));
  });

  it('returns different values for different passwords', () => {
    expect(hashPassword('salt', 'pass1')).not.toBe(hashPassword('salt', 'pass2'));
  });
});

describe('generateSalt', () => {
  it('returns a string of 16 characters', () => {
    expect(generateSalt()).toHaveLength(16);
  });

  it('only contains alphanumeric characters', () => {
    expect(generateSalt()).toMatch(/^[A-Za-z0-9]{16}$/);
  });

  it('returns different values on successive calls (not constant)', () => {
    const salts = new Set(Array.from({ length: 10 }, () => generateSalt()));
    expect(salts.size).toBeGreaterThan(1);
  });
});

describe('passwordSaltSupported', () => {
  it('returns non-zero when SupportsPasswordHash bit is set', () => {
    // SupportsPasswordHash = 2 from mock; 2 & 2 = 2
    expect(passwordSaltSupported(2)).toBeTruthy();
  });

  it('returns zero when SupportsPasswordHash bit is not set', () => {
    // 1 & 2 = 0
    expect(passwordSaltSupported(1)).toBeFalsy();
  });
});
