import Divider from '@mui/material/Divider';
import Menu from '@mui/material/Menu';
import MenuItem from '@mui/material/MenuItem';

import './PlayerContextMenu.css';

export interface PlayerContextMenuProps {
  isOpen: boolean;
  anchorPosition: { top: number; left: number } | null;
  onClose: () => void;
  onRequestCreateToken: () => void;
  onRequestViewSideboard: () => void;
}

function PlayerContextMenu({
  isOpen,
  anchorPosition,
  onClose,
  onRequestCreateToken,
  onRequestViewSideboard,
}: PlayerContextMenuProps) {
  const handleCreateToken = () => {
    onRequestCreateToken();
    onClose();
  };

  const handleViewSideboard = () => {
    onRequestViewSideboard();
    onClose();
  };

  return (
    <Menu
      open={isOpen}
      onClose={onClose}
      anchorReference="anchorPosition"
      anchorPosition={anchorPosition ?? undefined}
      data-testid="player-context-menu"
      className="player-context-menu"
    >
      <MenuItem onClick={handleCreateToken}>Create token…</MenuItem>
      <Divider />
      <MenuItem onClick={handleViewSideboard}>View sideboard…</MenuItem>
    </Menu>
  );
}

export default PlayerContextMenu;
