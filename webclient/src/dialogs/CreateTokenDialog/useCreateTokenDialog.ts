import { useEffect, useMemo, useState } from 'react';

import { TokenDTO } from '@app/services';

import type { CreateTokenSubmit } from './CreateTokenDialog';

export type ChooserScope = 'all' | 'deck';

export interface CreateTokenDialogState {
  name: string;
  color: string;
  pt: string;
  annotation: string;
  destroyOnZoneChange: boolean;
  faceDown: boolean;
  error: string | null;

  scope: ChooserScope;
  search: string;
  availableTokens: TokenDTO[];
  filteredTokens: TokenDTO[];
  selectedTokenName: string | null;

  setScope: (value: ChooserScope) => void;
  setSearch: (value: string) => void;
  selectPredefinedToken: (token: TokenDTO) => void;

  handleNameChange: (value: string) => void;
  setColor: (value: string) => void;
  setPT: (value: string) => void;
  setAnnotation: (value: string) => void;
  setDestroyOnZoneChange: (value: boolean) => void;
  setFaceDown: (value: boolean) => void;
  handleSubmit: (e?: React.FormEvent<HTMLFormElement>) => void;
}

export const CREATE_TOKEN_DEFAULT_COLOR = 'w';

// Desktop server-side MAX_NAME_LENGTH is 0xff (255). Client-side caps on
// the other free-text fields mirror that to prevent oversize-payload
// round-trips that the server would reject anyway.
export const MAX_NAME_LEN = 255;
export const MAX_PT_LEN = 255;
export const MAX_ANNOTATION_LEN = 255;

export interface UseCreateTokenDialogArgs {
  isOpen: boolean;
  onSubmit: (args: CreateTokenSubmit) => void;
  /** Optional deck-scoped token names; mirrors desktop DlgCreateToken predefinedTokens. */
  predefinedTokenNames?: string[];
}

/** Maps a MTGJSON-shaped color list ("W", "U", ...) to the dialog's single-letter color value. */
function colorFromToken(token: TokenDTO): string {
  const raw = token.prop?.value?.colors?.value ?? '';
  if (!raw) {
    return '';
  }
  const colors = raw.split(/[\s,]+/).filter(Boolean).map((c: string) => c.toLowerCase());
  if (colors.length === 0) {
    return '';
  }
  if (colors.length > 1) {
    return 'm';
  }
  const first = colors[0];
  if (first === 'w' || first === 'u' || first === 'b' || first === 'r' || first === 'g') {
    return first;
  }
  return '';
}

/** Best-effort providerId from the token's first set entry; matches desktop TokenInfo.providerId. */
function providerIdFromToken(token: TokenDTO): string | undefined {
  return token.set?.[0]?.value ?? undefined;
}

export function useCreateTokenDialog({
  isOpen,
  onSubmit,
  predefinedTokenNames,
}: UseCreateTokenDialogArgs): CreateTokenDialogState {
  const [name, setName] = useState('');
  const [color, setColor] = useState(CREATE_TOKEN_DEFAULT_COLOR);
  const [pt, setPT] = useState('');
  const [annotation, setAnnotation] = useState('');
  const [destroyOnZoneChange, setDestroyOnZoneChange] = useState(true);
  const [faceDown, setFaceDown] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const [scope, setScope] = useState<ChooserScope>(predefinedTokenNames?.length ? 'deck' : 'all');
  const [search, setSearch] = useState('');
  const [availableTokens, setAvailableTokens] = useState<TokenDTO[]>([]);
  const [selectedTokenName, setSelectedTokenName] = useState<string | null>(null);
  const [providerId, setProviderId] = useState<string | undefined>(undefined);

  useEffect(() => {
    if (isOpen) {
      setName('');
      setColor(CREATE_TOKEN_DEFAULT_COLOR);
      setPT('');
      setAnnotation('');
      setDestroyOnZoneChange(true);
      setFaceDown(false);
      setError(null);
      setSearch('');
      setSelectedTokenName(null);
      setProviderId(undefined);
      setScope(predefinedTokenNames?.length ? 'deck' : 'all');
    }
  }, [isOpen, predefinedTokenNames]);

  useEffect(() => {
    if (!isOpen) {
      return;
    }
    let cancelled = false;
    // Best-effort load of the token library. On failure the chooser renders
    // empty and freeform creation still works.
    import('@app/services').then(({ dexieService }) => {
      dexieService.tokens.toArray().then((tokens: TokenDTO[]) => {
        if (!cancelled) {
          setAvailableTokens(tokens);
        }
      }).catch(() => {
        if (!cancelled) {
          setAvailableTokens([]);
        }
      });
    });
    return () => {
      cancelled = true;
    };
  }, [isOpen]);

  const filteredTokens = useMemo(() => {
    const allowByScope = scope === 'deck' && predefinedTokenNames?.length
      ? new Set(predefinedTokenNames.map((n) => n.toLowerCase()))
      : null;
    const needle = search.trim().toLowerCase();
    return availableTokens.filter((token) => {
      const tokenName = token.name?.value ?? '';
      if (allowByScope && !allowByScope.has(tokenName.toLowerCase())) {
        return false;
      }
      if (needle && !tokenName.toLowerCase().includes(needle)) {
        return false;
      }
      return true;
    });
  }, [availableTokens, scope, search, predefinedTokenNames]);

  const handleNameChange = (value: string) => {
    setName(value.slice(0, MAX_NAME_LEN));
    if (error) {
      setError(null);
    }
  };

  const selectPredefinedToken = (token: TokenDTO) => {
    const tokenName = token.name?.value ?? '';
    setSelectedTokenName(tokenName);
    setName(tokenName.slice(0, MAX_NAME_LEN));
    setColor(colorFromToken(token));
    const ptRaw = token.prop?.value?.pt?.value ?? '';
    setPT(ptRaw.slice(0, MAX_PT_LEN));
    setProviderId(providerIdFromToken(token));
    if (error) {
      setError(null);
    }
  };

  const handleSubmit = (e?: React.FormEvent<HTMLFormElement>) => {
    e?.preventDefault();
    if (name.trim().length === 0) {
      setError('Name is required');
      return;
    }
    const payload: CreateTokenSubmit = {
      name: name.trim(),
      color,
      pt: pt.trim(),
      annotation: annotation.trim(),
      destroyOnZoneChange,
      faceDown,
    };
    if (providerId) {
      payload.providerId = providerId;
    }
    onSubmit(payload);
  };

  return {
    name,
    color,
    pt,
    annotation,
    destroyOnZoneChange,
    faceDown,
    error,
    scope,
    search,
    availableTokens,
    filteredTokens,
    selectedTokenName,
    setScope,
    setSearch,
    selectPredefinedToken,
    handleNameChange,
    setColor,
    setPT,
    setAnnotation,
    setDestroyOnZoneChange,
    setFaceDown,
    handleSubmit,
  };
}
