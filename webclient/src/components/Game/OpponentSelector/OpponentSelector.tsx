import { Select, MenuItem } from '@mui/material';

import './OpponentSelector.css';

export interface OpponentOption {
  playerId: number;
  name: string;
}

export interface OpponentSelectorProps {
  opponents: OpponentOption[];
  selectedPlayerId: number | undefined;
  onSelect: (playerId: number) => void;
}

function OpponentSelector({ opponents, selectedPlayerId, onSelect }: OpponentSelectorProps) {
  if (opponents.length < 2) {
    return null;
  }

  return (
    <div className="opponent-selector" data-testid="opponent-selector">
      <label className="opponent-selector__label">Opponent:</label>
      <Select
        className="opponent-selector__select"
        size="small"
        value={selectedPlayerId ?? ''}
        onChange={(e) => onSelect(Number(e.target.value))}
      >
        {opponents.map((o) => (
          <MenuItem key={o.playerId} value={o.playerId}>
            {o.name}
          </MenuItem>
        ))}
      </Select>
    </div>
  );
}

export default OpponentSelector;
