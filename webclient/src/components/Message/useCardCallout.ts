import { useMemo, useState } from 'react';

import { CardDTO, TokenDTO } from '@app/services';

export interface CardCallout {
  card: CardDTO | null;
  token: TokenDTO | null;
  anchorEl: Element | null;
  open: boolean;
  handlePopoverOpen: (event: React.MouseEvent) => void;
  handlePopoverClose: () => void;
}

export function useCardCallout(name: string): CardCallout {
  const [card, setCard] = useState<CardDTO | null>(null);
  const [token, setToken] = useState<TokenDTO | null>(null);
  const [anchorEl, setAnchorEl] = useState<Element | null>(null);

  useMemo(async () => {
    const c = await CardDTO.get(name);
    if (c) {
      return setCard(c);
    }

    const t = await TokenDTO.get(name);
    if (t) {
      return setToken(t);
    }
  }, [name]);

  const handlePopoverOpen = (event: React.MouseEvent) => {
    setAnchorEl(event.currentTarget);
  };

  const handlePopoverClose = () => {
    setAnchorEl(null);
  };

  const open = Boolean(anchorEl);

  return { card, token, anchorEl, open, handlePopoverOpen, handlePopoverClose };
}
