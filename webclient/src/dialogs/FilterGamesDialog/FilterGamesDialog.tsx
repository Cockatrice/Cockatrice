import { useEffect, useMemo, useState } from 'react';
import Button from '@mui/material/Button';
import Checkbox from '@mui/material/Checkbox';
import Dialog from '@mui/material/Dialog';
import DialogActions from '@mui/material/DialogActions';
import DialogContent from '@mui/material/DialogContent';
import DialogTitle from '@mui/material/DialogTitle';
import FormControl from '@mui/material/FormControl';
import FormControlLabel from '@mui/material/FormControlLabel';
import InputLabel from '@mui/material/InputLabel';
import MenuItem from '@mui/material/MenuItem';
import Select from '@mui/material/Select';
import TextField from '@mui/material/TextField';
import Typography from '@mui/material/Typography';

import { DEFAULT_GAME_FILTERS } from '@app/store';
import type { GameFilters } from '@app/store';
import type { Enriched } from '@app/types';

import './FilterGamesDialog.css';

export interface FilterGamesDialogProps {
  isOpen: boolean;
  /**
   * MUST be a stable reference across renders while the dialog is open.
   * The open-reset effect depends on `initialFilters` identity; an unstable
   * reference (e.g. `{ ...defaults }` freshly constructed every parent render)
   * will reset the draft form on every re-render. Pass a memoized value or a
   * module-level constant.
   */
  initialFilters: GameFilters;
  gametypeMap: Enriched.GametypeMap;
  onCancel: () => void;
  onSubmit: (filters: GameFilters) => void;
}

interface MaxAgeOption {
  label: string;
  seconds: number;
}

const MAX_AGE_OPTIONS: MaxAgeOption[] = [
  { label: 'No limit', seconds: 0 },
  { label: '5 minutes', seconds: 5 * 60 },
  { label: '10 minutes', seconds: 10 * 60 },
  { label: '30 minutes', seconds: 30 * 60 },
  { label: '1 hour', seconds: 60 * 60 },
  { label: '2 hours', seconds: 2 * 60 * 60 },
];

function FilterGamesDialog({
  isOpen,
  initialFilters,
  gametypeMap,
  onCancel,
  onSubmit,
}: FilterGamesDialogProps) {
  const [form, setForm] = useState<GameFilters>(initialFilters);
  const [creatorNamesText, setCreatorNamesText] = useState<string>(initialFilters.creatorNameFilters.join(', '));

  useEffect(() => {
    if (isOpen) {
      setForm(initialFilters);
      setCreatorNamesText(initialFilters.creatorNameFilters.join(', '));
    }
  }, [isOpen, initialFilters]);

  const gameTypes = useMemo(() => {
    return Object.entries(gametypeMap).map(([id, name]) => ({ id: Number(id), name }));
  }, [gametypeMap]);

  const update = <K extends keyof GameFilters>(key: K, value: GameFilters[K]) => {
    setForm((prev) => ({ ...prev, [key]: value }));
  };

  const toggleGameType = (id: number) => {
    setForm((prev) => {
      const set = new Set(prev.gameTypeFilter);
      if (set.has(id)) {
        set.delete(id);
      } else {
        set.add(id);
      }
      return { ...prev, gameTypeFilter: Array.from(set) };
    });
  };

  const handleSubmit = (e: React.FormEvent<HTMLFormElement>) => {
    e.preventDefault();
    const creatorNameFilters = creatorNamesText
      .split(',')
      .map((s) => s.trim())
      .filter(Boolean);
    onSubmit({ ...form, creatorNameFilters });
  };

  const handleReset = () => {
    setForm(DEFAULT_GAME_FILTERS);
    setCreatorNamesText('');
  };

  return (
    <Dialog className="FilterGamesDialog" open={isOpen} onClose={onCancel} maxWidth="sm" fullWidth>
      <DialogTitle>Filter games</DialogTitle>
      <form onSubmit={handleSubmit}>
        <DialogContent className="dialog-content filter-games-dialog__body">
          <TextField
            autoFocus
            fullWidth
            margin="dense"
            size="small"
            label="Game description contains"
            value={form.gameNameFilter}
            onChange={(e) => update('gameNameFilter', e.target.value)}
          />
          <TextField
            fullWidth
            margin="dense"
            size="small"
            label="Creator names (comma-separated)"
            value={creatorNamesText}
            onChange={(e) => setCreatorNamesText(e.target.value)}
          />
          <div className="filter-games-dialog__row">
            <TextField
              margin="dense"
              size="small"
              label="Min players"
              type="number"
              slotProps={{ htmlInput: { min: 0, max: 99 } }}
              value={form.maxPlayersFilterMin}
              onChange={(e) => update('maxPlayersFilterMin', Number(e.target.value))}
            />
            <TextField
              margin="dense"
              size="small"
              label="Max players"
              type="number"
              slotProps={{ htmlInput: { min: 0, max: 99 } }}
              value={form.maxPlayersFilterMax}
              onChange={(e) => update('maxPlayersFilterMax', Number(e.target.value))}
            />
          </div>

          <FormControl margin="dense" size="small" fullWidth>
            <InputLabel id="filter-games-max-age-label">Max age</InputLabel>
            <Select
              labelId="filter-games-max-age-label"
              label="Max age"
              value={form.maxGameAgeSeconds}
              onChange={(e) => update('maxGameAgeSeconds', Number(e.target.value))}
            >
              {MAX_AGE_OPTIONS.map((opt) => (
                <MenuItem key={opt.seconds} value={opt.seconds}>{opt.label}</MenuItem>
              ))}
            </Select>
          </FormControl>

          {gameTypes.length > 0 && (
            <div className="filter-games-dialog__section">
              <Typography variant="subtitle2">Game types</Typography>
              {gameTypes.map(({ id, name }) => (
                <FormControlLabel
                  key={id}
                  control={
                    <Checkbox
                      size="small"
                      checked={form.gameTypeFilter.includes(id)}
                      onChange={() => toggleGameType(id)}
                    />
                  }
                  label={name}
                />
              ))}
            </div>
          )}

          <div className="filter-games-dialog__section">
            <Typography variant="subtitle2">Hide</Typography>
            <FormControlLabel
              control={<Checkbox size="small" checked={form.hideFullGames} onChange={(_, c) => update('hideFullGames', c)} />}
              label="Hide full games"
            />
            <FormControlLabel
              control={<Checkbox size="small" checked={form.hideGamesThatStarted} onChange={(_, c) => update('hideGamesThatStarted', c)} />}
              label="Hide games that started"
            />
            <FormControlLabel
              control={
                <Checkbox
                  size="small"
                  checked={form.hidePasswordProtectedGames}
                  onChange={(_, c) => update('hidePasswordProtectedGames', c)}
                />
              }
              label="Hide password-protected games"
            />
            <FormControlLabel
              control={<Checkbox size="small" checked={form.hideBuddiesOnlyGames} onChange={(_, c) => update('hideBuddiesOnlyGames', c)} />}
              label="Hide buddies-only games"
            />
            <FormControlLabel
              control={<Checkbox size="small" checked={form.hideIgnoredUserGames} onChange={(_, c) => update('hideIgnoredUserGames', c)} />}
              label="Hide games created by ignored users"
            />
            <FormControlLabel
              control={
                <Checkbox
                  size="small"
                  checked={form.hideNotBuddyCreatedGames}
                  onChange={(_, c) => update('hideNotBuddyCreatedGames', c)}
                />
              }
              label="Hide games not created by buddies"
            />
            <FormControlLabel
              control={
                <Checkbox
                  size="small"
                  checked={form.hideOpenDecklistGames}
                  onChange={(_, c) => update('hideOpenDecklistGames', c)}
                />
              }
              label="Hide open-decklist games"
            />
          </div>

          <div className="filter-games-dialog__section">
            <Typography variant="subtitle2">Spectator filters</Typography>
            <FormControlLabel
              control={
                <Checkbox
                  size="small"
                  checked={form.showOnlyIfSpectatorsCanWatch}
                  onChange={(_, c) => update('showOnlyIfSpectatorsCanWatch', c)}
                />
              }
              label="Show only games where spectators can watch"
            />
            <FormControlLabel
              control={
                <Checkbox
                  size="small"
                  checked={form.showSpectatorPasswordProtected}
                  onChange={(_, c) => update('showSpectatorPasswordProtected', c)}
                  disabled={!form.showOnlyIfSpectatorsCanWatch}
                />
              }
              label="Show games where spectators need a password"
            />
            <FormControlLabel
              control={
                <Checkbox
                  size="small"
                  checked={form.showOnlyIfSpectatorsCanChat}
                  onChange={(_, c) => update('showOnlyIfSpectatorsCanChat', c)}
                  disabled={!form.showOnlyIfSpectatorsCanWatch}
                />
              }
              label="Show only games where spectators can chat"
            />
            <FormControlLabel
              control={
                <Checkbox
                  size="small"
                  checked={form.showOnlyIfSpectatorsCanSeeHands}
                  onChange={(_, c) => update('showOnlyIfSpectatorsCanSeeHands', c)}
                  disabled={!form.showOnlyIfSpectatorsCanWatch}
                />
              }
              label="Show only games where spectators see hands"
            />
          </div>
        </DialogContent>
        <DialogActions>
          <Button type="button" onClick={handleReset}>Reset</Button>
          <Button type="button" onClick={onCancel}>Cancel</Button>
          <Button type="submit" variant="contained" color="primary">Apply</Button>
        </DialogActions>
      </form>
    </Dialog>
  );
}

export default FilterGamesDialog;
