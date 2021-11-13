import sha512 from 'crypto-js/sha512';
import Base64 from 'crypto-js/enc-base64';

const HASH_ROUNDS = 1_000;
const SALT_LENGTH = 16;

export const hashPassword = (salt: string, password: string): string => {
  let hashedPassword = salt + password;
  for (let i = 0; i < HASH_ROUNDS; i++) {
    // WHY DO WE DO IT THIS WAY?
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
