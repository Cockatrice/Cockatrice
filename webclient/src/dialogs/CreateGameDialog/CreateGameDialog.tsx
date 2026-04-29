import { useEffect, useMemo, useState } from 'react';
import Button from '@mui/material/Button';
import Checkbox from '@mui/material/Checkbox';
import Dialog from '@mui/material/Dialog';
import DialogActions from '@mui/material/DialogActions';
import DialogContent from '@mui/material/DialogContent';
import DialogTitle from '@mui/material/DialogTitle';
import FormControlLabel from '@mui/material/FormControlLabel';
import Radio from '@mui/material/Radio';
import RadioGroup from '@mui/material/RadioGroup';
import TextField from '@mui/material/TextField';
import Typography from '@mui/material/Typography';

import { ServerSelectors, useAppSelector } from '@app/store';
import type { App, Enriched } from '@app/types';

import './CreateGameDialog.css';

export interface CreateGameDialogProps {
  isOpen: boolean;
  gametypeMap: Enriched.GametypeMap;
  onCancel: () => void;
  onSubmit: (params: App.CreateGameParams) => void;
}

const DEFAULT_MAX_PLAYERS = 2;
const DEFAULT_STARTING_LIFE = 20;

interface FormState {
  description: string;
  password: string;
  maxPlayers: number;
  onlyBuddies: boolean;
  onlyRegistered: boolean;
  spectatorsAllowed: boolean;
  spectatorsNeedPassword: boolean;
  spectatorsCanTalk: boolean;
  spectatorsSeeEverything: boolean;
  joinAsSpectator: boolean;
  startingLifeTotal: number;
  shareDecklistsOnLoad: boolean;
  joinAsJudge: boolean;
  gameTypeId: number | null;
}

function initialFormState(isRegistered: boolean): FormState {
  return {
    description: '',
    password: '',
    maxPlayers: DEFAULT_MAX_PLAYERS,
    onlyBuddies: false,
    onlyRegistered: isRegistered,
    spectatorsAllowed: true,
    spectatorsNeedPassword: false,
    spectatorsCanTalk: false,
    spectatorsSeeEverything: false,
    joinAsSpectator: false,
    startingLifeTotal: DEFAULT_STARTING_LIFE,
    shareDecklistsOnLoad: false,
    joinAsJudge: false,
    gameTypeId: null,
  };
}

function CreateGameDialog({ isOpen, gametypeMap, onCancel, onSubmit }: CreateGameDialogProps) {
  const isRegistered = useAppSelector(ServerSelectors.getIsUserRegistered);
  const isJudge = useAppSelector(ServerSelectors.getIsUserJudge);

  const gameTypes = useMemo(() => {
    return Object.entries(gametypeMap).map(([id, name]) => ({ id: Number(id), name }));
  }, [gametypeMap]);

  const [form, setForm] = useState<FormState>(() => initialFormState(isRegistered));

  useEffect(() => {
    if (isOpen) {
      setForm(initialFormState(isRegistered));
    }
  }, [isOpen, isRegistered]);

  const update = <K extends keyof FormState>(key: K, value: FormState[K]) => {
    setForm((prev) => ({ ...prev, [key]: value }));
  };

  const handleSubmit = (e: React.FormEvent<HTMLFormElement>) => {
    e.preventDefault();
    const params: App.CreateGameParams = {
      description: form.description,
      password: form.password,
      maxPlayers: form.maxPlayers,
      onlyBuddies: form.onlyBuddies,
      onlyRegistered: form.onlyRegistered,
      spectatorsAllowed: form.spectatorsAllowed,
      spectatorsNeedPassword: form.spectatorsAllowed && form.spectatorsNeedPassword,
      spectatorsCanTalk: form.spectatorsAllowed && form.spectatorsCanTalk,
      spectatorsSeeEverything: form.spectatorsAllowed && form.spectatorsSeeEverything,
      gameTypeIds: form.gameTypeId != null ? [form.gameTypeId] : [],
      joinAsJudge: isJudge && form.joinAsJudge,
      joinAsSpectator: form.joinAsSpectator,
      startingLifeTotal: form.startingLifeTotal,
      shareDecklistsOnLoad: form.shareDecklistsOnLoad,
    };
    onSubmit(params);
  };

  return (
    <Dialog className="CreateGameDialog" open={isOpen} onClose={onCancel} maxWidth="sm" fullWidth>
      <DialogTitle>Create Game</DialogTitle>
      <form onSubmit={handleSubmit}>
        <DialogContent className="dialog-content create-game-dialog__body">
          <TextField
            autoFocus
            fullWidth
            margin="dense"
            size="small"
            label="Description"
            value={form.description}
            onChange={(e) => update('description', e.target.value)}
          />
          <TextField
            fullWidth
            margin="dense"
            size="small"
            label="Password"
            type="password"
            value={form.password}
            onChange={(e) => update('password', e.target.value)}
          />
          <TextField
            fullWidth
            margin="dense"
            size="small"
            label="Max players"
            type="number"
            slotProps={{ htmlInput: { min: 1, max: 100 } }}
            value={form.maxPlayers}
            onChange={(e) => update('maxPlayers', Number(e.target.value))}
          />
          <TextField
            fullWidth
            margin="dense"
            size="small"
            label="Starting life total"
            type="number"
            slotProps={{ htmlInput: { min: 1, max: 99999 } }}
            value={form.startingLifeTotal}
            onChange={(e) => update('startingLifeTotal', Number(e.target.value))}
          />

          {gameTypes.length > 0 && (
            <div className="create-game-dialog__section">
              <Typography variant="subtitle2">Game type</Typography>
              <RadioGroup
                value={form.gameTypeId ?? ''}
                onChange={(_, value) => update('gameTypeId', value === '' ? null : Number(value))}
              >
                {gameTypes.map(({ id, name }) => (
                  <FormControlLabel key={id} value={id} control={<Radio size="small" />} label={name} />
                ))}
              </RadioGroup>
            </div>
          )}

          <div className="create-game-dialog__section">
            <Typography variant="subtitle2">Permissions</Typography>
            <FormControlLabel
              control={
                <Checkbox
                  size="small"
                  checked={form.onlyBuddies}
                  onChange={(_, c) => update('onlyBuddies', c)}
                  disabled={!isRegistered}
                />
              }
              label="Only buddies"
            />
            <FormControlLabel
              control={
                <Checkbox
                  size="small"
                  checked={form.onlyRegistered}
                  onChange={(_, c) => update('onlyRegistered', c)}
                />
              }
              label="Only registered users"
            />
          </div>

          <div className="create-game-dialog__section">
            <Typography variant="subtitle2">Spectators</Typography>
            <FormControlLabel
              control={
                <Checkbox
                  size="small"
                  checked={form.spectatorsAllowed}
                  onChange={(_, c) => update('spectatorsAllowed', c)}
                />
              }
              label="Allow spectators"
            />
            <FormControlLabel
              control={
                <Checkbox
                  size="small"
                  checked={form.spectatorsNeedPassword}
                  onChange={(_, c) => update('spectatorsNeedPassword', c)}
                  disabled={!form.spectatorsAllowed}
                />
              }
              label="Spectators need password"
            />
            <FormControlLabel
              control={
                <Checkbox
                  size="small"
                  checked={form.spectatorsCanTalk}
                  onChange={(_, c) => update('spectatorsCanTalk', c)}
                  disabled={!form.spectatorsAllowed}
                />
              }
              label="Spectators can chat"
            />
            <FormControlLabel
              control={
                <Checkbox
                  size="small"
                  checked={form.spectatorsSeeEverything}
                  onChange={(_, c) => update('spectatorsSeeEverything', c)}
                  disabled={!form.spectatorsAllowed}
                />
              }
              label="Spectators see everything"
            />
            <FormControlLabel
              control={
                <Checkbox
                  size="small"
                  checked={form.joinAsSpectator}
                  onChange={(_, c) => update('joinAsSpectator', c)}
                />
              }
              label="Create as spectator"
            />
          </div>

          <div className="create-game-dialog__section">
            <Typography variant="subtitle2">Other</Typography>
            <FormControlLabel
              control={
                <Checkbox
                  size="small"
                  checked={form.shareDecklistsOnLoad}
                  onChange={(_, c) => update('shareDecklistsOnLoad', c)}
                />
              }
              label="Share decklists on load"
            />
            {isJudge && (
              <FormControlLabel
                control={
                  <Checkbox
                    size="small"
                    checked={form.joinAsJudge}
                    onChange={(_, c) => update('joinAsJudge', c)}
                  />
                }
                label="Create as judge"
              />
            )}
          </div>
        </DialogContent>
        <DialogActions>
          <Button type="button" onClick={onCancel}>Cancel</Button>
          <Button type="submit" variant="contained" color="primary">Create</Button>
        </DialogActions>
      </form>
    </Dialog>
  );
}

export default CreateGameDialog;
