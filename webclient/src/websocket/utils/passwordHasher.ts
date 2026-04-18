import sha512 from 'crypto-js/sha512';
import Base64 from 'crypto-js/enc-base64';
import { Event_ServerIdentification_ServerOptions } from '@app/generated';

const HASH_ROUNDS = 1_000;
const SALT_LENGTH = 16;

export const hashPassword = (salt: string, password: string): string => {
  let hashedPassword = salt + password;
  for (let i = 0; i < HASH_ROUNDS; i++) {
    hashedPassword = sha512(hashedPassword);
  }

  return salt + Base64.stringify(hashedPassword);
};

export const generateSalt = (): string => {
  const characters = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'

  let salt = '';
  for (let i = 0; i < SALT_LENGTH; i++) {
    salt += characters.charAt(Math.floor(Math.random() * characters.length));
  }

  return salt;
}

export const passwordSaltSupported = (serverOptions: number): number => {
  // @critical Servatrice ServerOptions is a bitmask. See .github/instructions/webclient.instructions.md#protocol-quirks.
  return serverOptions & Event_ServerIdentification_ServerOptions.SupportsPasswordHash;
}
