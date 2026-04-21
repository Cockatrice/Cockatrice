vi.mock('../../generated/proto/event_server_identification_pb', async (importOriginal) => ({
  ...(await importOriginal<typeof import('../../generated/proto/event_server_identification_pb')>()),
  Event_ServerIdentification_ServerOptions: { SupportsPasswordHash: 1 },
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
  it('returns false for NoOptions (0)', () => {
    expect(passwordSaltSupported(0)).toBeFalsy();
  });

  it('returns true when the SupportsPasswordHash bit (1) is set', () => {
    expect(passwordSaltSupported(1)).toBeTruthy();
  });

  it('returns false when an unrelated bit is set (2)', () => {
    expect(passwordSaltSupported(2)).toBeFalsy();
  });

  it('returns true when bit 0 is set alongside other bits (3)', () => {
    expect(passwordSaltSupported(3)).toBeTruthy();
  });
});
