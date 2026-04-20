import { createRef } from 'react';
import { act, renderHook } from '@testing-library/react';

import { createCardRegistry } from '../../components/Game/CardRegistry/CardRegistryContext';
import { combineReducers } from '@reduxjs/toolkit';

import { gamesReducer } from '../../store/game/game.reducer';
import { makeGameEntry, makePlayerEntry, makePlayerProperties } from '../../store/game/__mocks__/fixtures';
import type { GamesState } from '../../store/game/game.interfaces';
import { makeReduxWebClientHookWrapper } from '../../__test-utils__/makeHookWrapper';
import { App } from '../../types';
import { useGameArrowInteractions } from './useGameArrowInteractions';

function setup({ localPlayerId = 1 }: { localPlayerId?: number } = {}) {
  const game = makeGameEntry({
    localPlayerId,
    players: {
      [localPlayerId]: makePlayerEntry({
        properties: makePlayerProperties({ playerId: localPlayerId }),
      }),
    },
  });
  const gamesState: GamesState = { games: { 1: { ...game, info: { ...game.info, gameId: 1 } } } };

  const { Wrapper, webClient } = makeReduxWebClientHookWrapper({
    reducer: combineReducers({ games: gamesReducer }),
    preloadedState: { games: gamesState },
  });

  const boardRef = createRef<HTMLDivElement>();
  const board = document.createElement('div');
  board.getBoundingClientRect = () =>
    ({ left: 0, top: 0, right: 1000, bottom: 1000, width: 1000, height: 1000, x: 0, y: 0, toJSON: () => ({}) }) as DOMRect;
  (boardRef as { current: HTMLDivElement | null }).current = board;

  const cardRegistry = createCardRegistry();

  const { result } = renderHook(
    () =>
      useGameArrowInteractions({
        gameId: 1,
        game: { ...game, info: { ...game.info, gameId: 1 } },
        boardRef,
        cardRegistry,
      }),
    { wrapper: Wrapper },
  );

  return { result, webClient, boardRef };
}

function makeCardElement({
  playerId,
  zone,
  cardId,
}: {
  playerId: number;
  zone: string;
  cardId: number;
}): HTMLElement {
  const el = document.createElement('div');
  el.setAttribute('data-card-id', String(cardId));
  el.setAttribute('data-card-owner', String(playerId));
  el.setAttribute('data-card-zone', zone);
  document.body.appendChild(el);
  return el;
}

function fireMouseEvent(type: string, init: Partial<MouseEventInit> = {}) {
  window.dispatchEvent(new MouseEvent(type, { bubbles: true, ...init }));
}

describe('useGameArrowInteractions', () => {
  afterEach(() => {
    document.body.innerHTML = '';
  });

  it('creates an arrow after right-click-drag past the 8px threshold', () => {
    const { result, webClient } = setup();
    const targetEl = makeCardElement({ playerId: 2, zone: App.ZoneName.TABLE, cardId: 99 });
    const origElementFromPoint = document.elementFromPoint;
    document.elementFromPoint = () => targetEl;

    act(() => {
      result.current.handleBoardMouseDown({
        button: 2,
        clientX: 10,
        clientY: 10,
        target: makeCardElement({ playerId: 1, zone: App.ZoneName.TABLE, cardId: 5 }),
      } as unknown as React.MouseEvent<HTMLDivElement>);
    });

    act(() => {
      fireMouseEvent('mousemove', { clientX: 30, clientY: 30 });
    });

    act(() => {
      fireMouseEvent('mouseup', { button: 2, clientX: 30, clientY: 30 });
    });

    expect(webClient.request.game.createArrow).toHaveBeenCalledWith(
      1,
      expect.objectContaining({
        startPlayerId: 1,
        startCardId: 5,
        targetPlayerId: 2,
        targetCardId: 99,
        targetZone: App.ZoneName.TABLE,
      }),
    );

    document.elementFromPoint = origElementFromPoint;
  });

  it('plays the card (moveCard) when dragging from HAND to a non-HAND target', () => {
    const { result, webClient } = setup({ localPlayerId: 1 });
    const targetEl = makeCardElement({ playerId: 2, zone: App.ZoneName.TABLE, cardId: 99 });
    const origElementFromPoint = document.elementFromPoint;
    document.elementFromPoint = () => targetEl;

    act(() => {
      result.current.handleBoardMouseDown({
        button: 2,
        clientX: 0,
        clientY: 0,
        target: makeCardElement({ playerId: 1, zone: App.ZoneName.HAND, cardId: 5 }),
      } as unknown as React.MouseEvent<HTMLDivElement>);
    });
    act(() => fireMouseEvent('mousemove', { clientX: 30, clientY: 30 }));
    act(() => fireMouseEvent('mouseup', { button: 2, clientX: 30, clientY: 30 }));

    expect(webClient.request.game.moveCard).toHaveBeenCalled();
    expect(webClient.request.game.createArrow).not.toHaveBeenCalled();

    document.elementFromPoint = origElementFromPoint;
  });

  it('does not send a request when the drop lands on the same card (cancel)', () => {
    const { result, webClient } = setup();
    const sameEl = makeCardElement({ playerId: 1, zone: App.ZoneName.TABLE, cardId: 5 });
    const origElementFromPoint = document.elementFromPoint;
    document.elementFromPoint = () => sameEl;

    act(() => {
      result.current.handleBoardMouseDown({
        button: 2,
        clientX: 0,
        clientY: 0,
        target: sameEl,
      } as unknown as React.MouseEvent<HTMLDivElement>);
    });
    act(() => fireMouseEvent('mousemove', { clientX: 30, clientY: 30 }));
    act(() => fireMouseEvent('mouseup', { button: 2, clientX: 30, clientY: 30 }));

    expect(webClient.request.game.createArrow).not.toHaveBeenCalled();

    document.elementFromPoint = origElementFromPoint;
  });

  it('does not send a request when mouseup is below the drag threshold', () => {
    const { result, webClient } = setup();
    const targetEl = makeCardElement({ playerId: 2, zone: App.ZoneName.TABLE, cardId: 99 });
    const origElementFromPoint = document.elementFromPoint;
    document.elementFromPoint = () => targetEl;

    act(() => {
      result.current.handleBoardMouseDown({
        button: 2,
        clientX: 10,
        clientY: 10,
        target: makeCardElement({ playerId: 1, zone: App.ZoneName.TABLE, cardId: 5 }),
      } as unknown as React.MouseEvent<HTMLDivElement>);
    });
    act(() => fireMouseEvent('mouseup', { button: 2, clientX: 12, clientY: 12 }));

    expect(webClient.request.game.createArrow).not.toHaveBeenCalled();
    expect(webClient.request.game.moveCard).not.toHaveBeenCalled();

    document.elementFromPoint = origElementFromPoint;
  });

  it('ESC cancels pending arrow state', () => {
    const { result } = setup();

    act(() => {
      result.current.startPendingArrow({ sourcePlayerId: 1, sourceZone: App.ZoneName.TABLE, sourceCardId: 5 });
    });
    expect(result.current.arrowSourceKey).not.toBeNull();

    act(() => {
      window.dispatchEvent(new KeyboardEvent('keydown', { key: 'Escape' }));
    });
    expect(result.current.arrowSourceKey).toBeNull();
  });

  it('ESC does not cancel while a MUI dialog is open', () => {
    const { result } = setup();

    const dialog = document.createElement('div');
    dialog.className = 'MuiDialog-root';
    dialog.setAttribute('role', 'dialog');
    document.body.appendChild(dialog);

    act(() => {
      result.current.startPendingArrow({ sourcePlayerId: 1, sourceZone: App.ZoneName.TABLE, sourceCardId: 5 });
    });
    act(() => {
      window.dispatchEvent(new KeyboardEvent('keydown', { key: 'Escape' }));
    });

    expect(result.current.arrowSourceKey).not.toBeNull();
  });
});
