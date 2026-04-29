import { TokenDTO } from '@app/services';

import './Token.css';

interface TokenProps {
  token: TokenDTO;
}

const Token = ({ token }: TokenProps) => {
  if (!token) {
    return null;
  }
  const set = Array.isArray(token.set) ? token.set[0] : token.set;
  return <img className="token" src={set?.picURL} alt={token.name?.value} />;
};

export default Token;
