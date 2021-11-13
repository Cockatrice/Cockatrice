export class Card {
  artist: string;
  availability: string[];
  borderColor: string;
  colorIdentity: string[];
  colors: string[];
  convertedManaCost: number;
  edhrecRank: number;
  flavorText: string;
  identifiers: {
    cardKingdomId: string;
    mcmId: string;
    mcmMetaId: string;
    mtgjsonV4Id: string;
    multiverseId: string;
    scryfallId: string;
    scryfallIllustrationId: string;
    scryfallOracleId: string;
    tcgplayerProductId: string;
  };
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
  side: string;
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
  name: { value: string };
  prop: {
    value: {
      cmc: { value: string; };
      colors: { value: string; };
      maintype: { value: string; };
      pt: { value: string; };
      type: { value: string; };
    };
  };
  related: { value: string; }[];
  'reverse-related': { value: string; }[];
  set: {
    value: string;
    picURL: string;
  }[];
  tablerow: { value: string; };
  text: { value: string; };
}
