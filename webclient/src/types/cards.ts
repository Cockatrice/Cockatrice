export class Card {
  artist: string;
  availability: string[];
  borderColor: string;
  colorIdentity: string[];
  colors: string[];
  convertedManaCost: number;
  edhrecRank: number;
  flavorText: string;
  identifiers: string[];
  isOnlineOnly: boolean;
  layout: string;
  legalities: {
    brawl: string;
    commander: string;
    duel: string;
    future: string;
    gladiator: string;
    historic: string;
    legacy: string;
    modern: string;
    pauper: string;
    penny: string;
    pioneer: string;
    premodern: string;
    standard: string;
    vintage: string;
  };
  manaCost: string;
  name: string;
  originalText: string;
  originalType: string;
  power: string;
  printings: string[];
  rarity: string;
  rulings: {
  	date: string;
  	text: string;
  }[];
  setCode: string;
  subtypes: string[];
  supertypes: string[];
  text: string;
  toughness: string;
  type: string;
  types: string[];
  uuid: string;
  variations: string[];
}

export class Set {
  baseSetSize: number;
  block: string;
  cards: string[];
  code: string;
  isOnlineOnly: boolean;
  name: string;
  releaseDate: string;
  totalSetSize: number;
  type: string;
}

export class Token {
  name: object;
  prop: object;
  'reverse-related': object;
  set: object;
  tablerow: object;
  text: object;
}
