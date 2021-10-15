// eslint-disable-next-line
import React, { useMemo, useState } from 'react';

import { TokenDTO } from 'services';

import './Token.css';

interface TokenProps {
  token: TokenDTO;
}

const Token = ({ token }: TokenProps) => {
  const set = Array.isArray(token?.set) ? token?.set[0] : token?.set;
  return token && (
    <img className="token" src={set?.picURL} alt={token?.name?.value} />
  );
}

export default Token;
