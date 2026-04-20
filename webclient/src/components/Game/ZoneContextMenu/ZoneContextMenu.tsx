import Menu from '@mui/material/Menu';
import MenuItem from '@mui/material/MenuItem';
import Divider from '@mui/material/Divider';
import Check from '@mui/icons-material/Check';

import { useWebClient } from '@app/hooks';
import { GameSelectors, useAppSelector } from '@app/store';
import { App } from '@app/types';

import './ZoneContextMenu.css';

export interface ZoneContextMenuProps {
  isOpen: boolean;
  anchorPosition: { top: number; left: number } | null;
  gameId: number;
  playerId: number | null;
  zoneName: string | null;
  onClose: () => void;
  onRequestDrawN: () => void;
  onRequestDumpN: () => void;
  onRequestRevealTopN: () => void;
  onRequestRevealZone: () => void;
}

function ZoneContextMenu({
  isOpen,
  anchorPosition,
  gameId,
  playerId,
  zoneName,
  onClose,
  onRequestDrawN,
  onRequestDumpN,
  onRequestRevealTopN,
  onRequestRevealZone,
}: ZoneContextMenuProps) {
  const webClient = useWebClient();

  const zone = useAppSelector((state) =>
    playerId != null && zoneName != null
      ? GameSelectors.getZone(state, gameId, playerId, zoneName)
      : undefined,
  );

  if (playerId == null || zoneName == null) {
    return null;
  }

  const game = webClient.request.game;
  const alwaysReveal = zone?.alwaysRevealTopCard ?? false;
  const alwaysLook = zone?.alwaysLookAtTopCard ?? false;

  // Close-then-act helpers (avoid duplicating onClose at every site).
  const run = (fn: () => void) => () => {
    fn();
    onClose();
  };

  const handleDrawOne = () => {
    game.drawCards(gameId, { number: 1 });
  };

  const handleShuffle = () => {
    game.shuffle(gameId, { zoneName: App.ZoneName.DECK, start: 0, end: -1 });
  };

  const handleRevealTop = () => {
    game.revealCards(gameId, {
      zoneName: App.ZoneName.DECK,
      playerId: -1,
      topCards: 1,
    });
  };

  const handleToggleAlwaysReveal = () => {
    game.changeZoneProperties(gameId, {
      zoneName: App.ZoneName.DECK,
      alwaysRevealTopCard: !alwaysReveal,
    });
  };

  const handleToggleAlwaysLook = () => {
    game.changeZoneProperties(gameId, {
      zoneName: App.ZoneName.DECK,
      alwaysLookAtTopCard: !alwaysLook,
    });
  };

  const menuItems: React.ReactNode[] = [];

  if (zoneName === App.ZoneName.DECK) {
    menuItems.push(
      <MenuItem key="draw-one" onClick={run(handleDrawOne)}>Draw a card</MenuItem>,
      <MenuItem key="draw-n" onClick={run(onRequestDrawN)}>Draw N cards…</MenuItem>,
      <MenuItem key="shuffle" onClick={run(handleShuffle)}>Shuffle</MenuItem>,
      <MenuItem key="dump-n" onClick={run(onRequestDumpN)}>Dump top N…</MenuItem>,
      <Divider key="d1" />,
      <MenuItem key="reveal-top" onClick={run(handleRevealTop)}>
        Reveal top card to all
      </MenuItem>,
      <MenuItem key="reveal-top-n" onClick={run(onRequestRevealTopN)}>
        Reveal top N to…
      </MenuItem>,
      <Divider key="d2" />,
      <MenuItem
        key="always-reveal"
        onClick={run(handleToggleAlwaysReveal)}
        className="zone-context-menu__toggle"
      >
        <span className="zone-context-menu__check" aria-hidden>
          {alwaysReveal ? <Check fontSize="inherit" /> : null}
        </span>
        Always reveal top card
      </MenuItem>,
      <MenuItem
        key="always-look"
        onClick={run(handleToggleAlwaysLook)}
        className="zone-context-menu__toggle"
      >
        <span className="zone-context-menu__check" aria-hidden>
          {alwaysLook ? <Check fontSize="inherit" /> : null}
        </span>
        Always look at top card
      </MenuItem>,
    );
  } else if (zoneName === App.ZoneName.GRAVE) {
    menuItems.push(
      <MenuItem key="reveal-grave" onClick={run(onRequestRevealZone)}>
        Reveal graveyard to…
      </MenuItem>,
    );
  } else if (zoneName === App.ZoneName.EXILE) {
    menuItems.push(
      <MenuItem key="reveal-exile" onClick={run(onRequestRevealZone)}>
        Reveal exile to…
      </MenuItem>,
    );
  } else {
    return null;
  }

  return (
    <Menu
      open={isOpen}
      onClose={onClose}
      anchorReference="anchorPosition"
      anchorPosition={anchorPosition ?? undefined}
      data-testid="zone-context-menu"
      className="zone-context-menu"
    >
      {menuItems}
    </Menu>
  );
}

export default ZoneContextMenu;
