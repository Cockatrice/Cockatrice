import Menu from '@mui/material/Menu';
import MenuItem from '@mui/material/MenuItem';
import Divider from '@mui/material/Divider';
import Check from '@mui/icons-material/Check';

import { App } from '@app/types';

import { useZoneContextMenu } from './useZoneContextMenu';

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

function ZoneContextMenu(props: ZoneContextMenuProps) {
  const {
    isOpen,
    anchorPosition,
    zoneName,
    onClose,
    onRequestDrawN,
    onRequestDumpN,
    onRequestRevealTopN,
    onRequestRevealZone,
  } = props;
  const {
    ready,
    alwaysReveal,
    alwaysLook,
    handleDrawOne,
    handleShuffle,
    handleRevealTop,
    handleToggleAlwaysReveal,
    handleToggleAlwaysLook,
    runAndClose,
  } = useZoneContextMenu(props);

  if (!ready) {
    return null;
  }

  const menuItems: React.ReactNode[] = [];

  if (zoneName === App.ZoneName.DECK) {
    menuItems.push(
      <MenuItem key="draw-one" onClick={runAndClose(handleDrawOne)}>Draw a card</MenuItem>,
      <MenuItem key="draw-n" onClick={runAndClose(onRequestDrawN)}>Draw N cards…</MenuItem>,
      <MenuItem key="shuffle" onClick={runAndClose(handleShuffle)}>Shuffle</MenuItem>,
      <MenuItem key="dump-n" onClick={runAndClose(onRequestDumpN)}>Dump top N…</MenuItem>,
      <Divider key="d1" />,
      <MenuItem key="reveal-top" onClick={runAndClose(handleRevealTop)}>
        Reveal top card to all
      </MenuItem>,
      <MenuItem key="reveal-top-n" onClick={runAndClose(onRequestRevealTopN)}>
        Reveal top N to…
      </MenuItem>,
      <Divider key="d2" />,
      <MenuItem
        key="always-reveal"
        onClick={runAndClose(handleToggleAlwaysReveal)}
        className="zone-context-menu__toggle"
      >
        <span className="zone-context-menu__check" aria-hidden>
          {alwaysReveal ? <Check fontSize="inherit" /> : null}
        </span>
        Always reveal top card
      </MenuItem>,
      <MenuItem
        key="always-look"
        onClick={runAndClose(handleToggleAlwaysLook)}
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
      <MenuItem key="reveal-grave" onClick={runAndClose(onRequestRevealZone)}>
        Reveal graveyard to…
      </MenuItem>,
    );
  } else if (zoneName === App.ZoneName.EXILE) {
    menuItems.push(
      <MenuItem key="reveal-exile" onClick={runAndClose(onRequestRevealZone)}>
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
