import Button from '@mui/material/Button';
import FilterListIcon from '@mui/icons-material/FilterList';
import FilterListOffIcon from '@mui/icons-material/FilterListOff';

export interface GameSelectorToolbarProps {
  isFilterActive: boolean;
  canCreate: boolean;
  canJoin: boolean;
  canSpectate: boolean;
  isJudgeUser: boolean;
  onFilter: () => void;
  onClearFilter: () => void;
  onCreate: () => void;
  onJoin: () => void;
  onSpectate: () => void;
  onJoinAsJudge: () => void;
  onSpectateAsJudge: () => void;
}

const GameSelectorToolbar = (props: GameSelectorToolbarProps) => {
  const {
    isFilterActive,
    canCreate,
    canJoin,
    canSpectate,
    isJudgeUser,
    onFilter,
    onClearFilter,
    onCreate,
    onJoin,
    onSpectate,
    onJoinAsJudge,
    onSpectateAsJudge,
  } = props;

  return (
    <div className="game-selector__toolbar">
      <div className="game-selector__toolbar-left">
        <Button
          size="small"
          variant={isFilterActive ? 'contained' : 'outlined'}
          color="primary"
          startIcon={<FilterListIcon />}
          onClick={onFilter}
        >
          Filter games
        </Button>
        <Button
          size="small"
          variant="outlined"
          startIcon={<FilterListOffIcon />}
          onClick={onClearFilter}
          disabled={!isFilterActive}
        >
          Clear filter
        </Button>
      </div>
      <div className="game-selector__toolbar-right">
        <Button size="small" variant="outlined" onClick={onCreate} disabled={!canCreate}>
          Create
        </Button>
        <Button size="small" variant="outlined" onClick={onJoin} disabled={!canJoin}>
          Join
        </Button>
        <Button size="small" variant="outlined" onClick={onSpectate} disabled={!canSpectate}>
          Join as Spectator
        </Button>
        {isJudgeUser && (
          <>
            <Button size="small" variant="outlined" onClick={onJoinAsJudge} disabled={!canJoin}>
              Join as Judge
            </Button>
            <Button
              size="small"
              variant="outlined"
              onClick={onSpectateAsJudge}
              disabled={!canSpectate}
            >
              Join as Judge Spectator
            </Button>
          </>
        )}
      </div>
    </div>
  );
};

export default GameSelectorToolbar;
