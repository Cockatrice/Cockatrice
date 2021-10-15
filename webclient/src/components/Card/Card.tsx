// eslint-disable-next-line
import React, { useMemo, useState } from 'react';

import { CardDTO } from 'services';

import './Card.css';

interface CardProps {
  card: CardDTO;
}

const Card = ({ card }: CardProps) => {
  const src = `https://api.scryfall.com/cards/${card?.identifiers?.scryfallId}?format=image`;

  return card && (
    <img className="card" src={src} alt={card?.name} />
  );
}

export default Card;
