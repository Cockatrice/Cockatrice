// eslint-disable-next-line
import React, { useMemo, useState } from 'react';

import { CardDTO } from 'services';

import './Card.css';

const Card = ({ name }) => {
  const [card, setCard] = useState(null);

  useMemo(() => {
    CardDTO.get(name).then(card => setCard(card));
  }, [name]);

  const src = `https://api.scryfall.com/cards/multiverse/${card?.identifiers?.multiverseId}?format=image`;

  return card && (
    <img className="card" src={src} alt={card?.name || name} />
  );
}

export default Card;
