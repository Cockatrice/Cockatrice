import { create } from '@bufbuild/protobuf';
import { Data } from '@app/types';

import {
  diffPlayerProperties,
  formatActivePhaseSet,
  formatActivePlayerSet,
  formatArrowCreated,
  formatCardAttached,
  formatCardAttrChanged,
  formatCardCounterChanged,
  formatCardDestroyed,
  formatCardFlipped,
  formatCardMoved,
  formatCardsDrawn,
  formatCounterSet,
  formatDieRolled,
  formatPlayerJoined,
  formatPropertyDiff,
  formatTokenCreated,
  formatTurnReversed,
  formatZoneDumped,
  formatZonePropertiesChanged,
  formatZoneShuffled,
} from './messageLog';
import {
  makeArrow,
  makeCard,
  makeGameEntry,
  makePlayerEntry,
  makePlayerProperties,
  makeZoneEntry,
} from './__mocks__/fixtures';

function gameWithTwoPlayers() {
  return makeGameEntry({
    localPlayerId: 1,
    activePlayerId: 1,
    players: {
      1: makePlayerEntry({
        properties: makePlayerProperties({ playerId: 1, userInfo: { name: 'Alice' } }),
        zones: {
          hand: makeZoneEntry({ name: 'hand' }),
          table: makeZoneEntry({ name: 'table', cards: [makeCard({ id: 10, name: 'Bolt' })], cardCount: 1 }),
        },
      }),
      2: makePlayerEntry({
        properties: makePlayerProperties({ playerId: 2, userInfo: { name: 'Bob' } }),
        zones: {
          hand: makeZoneEntry({ name: 'hand' }),
          table: makeZoneEntry({ name: 'table', cards: [makeCard({ id: 20, name: 'Bear' })], cardCount: 1 }),
        },
      }),
    },
  });
}

describe('formatCardMoved', () => {
  const game = gameWithTwoPlayers();

  it('logs hand → battlefield as "plays"', () => {
    const msg = formatCardMoved(game, 1, {
      cardId: 5, cardName: 'Bolt',
      startPlayerId: 1, startZone: 'hand',
      targetPlayerId: 1, targetZone: 'table',
      position: -1, x: 0, y: 0, newCardId: -1, faceDown: false, newCardProviderId: '',
    }, { resolvedCardName: 'Bolt' });
    expect(msg).toBe('Alice plays Bolt.');
  });

  it('logs library → hand as "puts into their hand"', () => {
    const msg = formatCardMoved(game, 1, {
      cardId: 5, cardName: 'Mystery',
      startPlayerId: 1, startZone: 'deck',
      targetPlayerId: 1, targetZone: 'hand',
      position: 0, x: 0, y: 0, newCardId: -1, faceDown: false, newCardProviderId: '',
    }, { resolvedCardName: '' });
    expect(msg).toBe('Alice puts Mystery into their hand.');
  });

  it('returns null for same-owner table-to-table reorder', () => {
    const msg = formatCardMoved(game, 1, {
      cardId: 5, cardName: 'Bolt',
      startPlayerId: 1, startZone: 'table',
      targetPlayerId: 1, targetZone: 'table',
      position: -1, x: 0, y: 0, newCardId: -1, faceDown: false, newCardProviderId: '',
    }, { resolvedCardName: 'Bolt' });
    expect(msg).toBeNull();
  });

  it('logs cross-owner control transfer', () => {
    const msg = formatCardMoved(game, 1, {
      cardId: 5, cardName: 'Bolt',
      startPlayerId: 1, startZone: 'table',
      targetPlayerId: 2, targetZone: 'table',
      position: -1, x: 0, y: 0, newCardId: -1, faceDown: false, newCardProviderId: '',
    }, { resolvedCardName: 'Bolt' });
    expect(msg).toBe('Alice gives Bob control over Bolt.');
  });

  it('returns null for hand-to-hand (sideboard reorder)', () => {
    const msg = formatCardMoved(game, 1, {
      cardId: 5, cardName: 'Bolt',
      startPlayerId: 1, startZone: 'hand',
      targetPlayerId: 1, targetZone: 'hand',
      position: -1, x: 0, y: 0, newCardId: -1, faceDown: false, newCardProviderId: '',
    }, { resolvedCardName: 'Bolt' });
    expect(msg).toBeNull();
  });
});

describe('formatCardFlipped', () => {
  const game = gameWithTwoPlayers();
  it('logs face-up with revealed name', () => {
    const msg = formatCardFlipped(game, 1, {
      zoneName: 'hand', cardId: 5, cardName: 'Bolt', faceDown: false, cardProviderId: '',
    }, '');
    expect(msg).toBe('Alice flips Bolt face-up.');
  });
  it('logs face-down with prior name', () => {
    const msg = formatCardFlipped(game, 1, {
      zoneName: 'hand', cardId: 5, cardName: '', faceDown: true, cardProviderId: '',
    }, 'Bolt');
    expect(msg).toBe('Alice flips Bolt face-down.');
  });
});

describe('formatCardDestroyed', () => {
  it('logs destruction with card name', () => {
    const game = gameWithTwoPlayers();
    expect(formatCardDestroyed(game, 1, 'Bolt')).toBe('Alice destroys Bolt.');
  });
  it('falls back to "a card" when name unknown', () => {
    const game = gameWithTwoPlayers();
    expect(formatCardDestroyed(game, 1, '')).toBe('Alice destroys a card.');
  });
});

describe('formatCardAttached', () => {
  const game = gameWithTwoPlayers();
  it('logs attach to opponent card', () => {
    const msg = formatCardAttached(game, 1, {
      startZone: 'table', cardId: 10,
      targetPlayerId: 2, targetZone: 'table', targetCardId: 20,
    }, 'Bolt');
    expect(msg).toBe('Alice attaches Bolt to Bob\'s Bear.');
  });
  it('logs unattach when target cleared', () => {
    const msg = formatCardAttached(game, 1, {
      startZone: 'table', cardId: 10,
      targetPlayerId: -1, targetZone: '', targetCardId: -1,
    }, 'Bolt');
    expect(msg).toBe('Alice unattaches Bolt.');
  });
});

describe('formatTokenCreated', () => {
  const game = gameWithTwoPlayers();
  it('logs token with name and PT', () => {
    const msg = formatTokenCreated(game, 1, {
      zoneName: 'table', cardId: 99, cardName: 'Goblin',
      color: 'r', pt: '1/1', annotation: '',
      destroyOnZoneChange: true, x: 0, y: 0, cardProviderId: '', faceDown: false,
    });
    expect(msg).toBe('Alice creates token: Goblin (1/1).');
  });
  it('logs face-down token distinctly', () => {
    const msg = formatTokenCreated(game, 1, {
      zoneName: 'table', cardId: 99, cardName: '',
      color: '', pt: '', annotation: '',
      destroyOnZoneChange: true, x: 0, y: 0, cardProviderId: '', faceDown: true,
    });
    expect(msg).toBe('Alice creates a face-down token.');
  });
});

describe('formatCardAttrChanged', () => {
  const game = gameWithTwoPlayers();
  it.each([
    [Data.CardAttribute.AttrTapped, '1', 'Alice taps Bolt.'],
    [Data.CardAttribute.AttrTapped, '0', 'Alice untaps Bolt.'],
    [Data.CardAttribute.AttrPT, '2/3', 'Alice sets PT of Bolt to 2/3.'],
    [Data.CardAttribute.AttrAnnotation, 'note', 'Alice sets annotation of Bolt to "note".'],
    [Data.CardAttribute.AttrDoesntUntap, '1', 'Alice sets Bolt to not untap normally.'],
  ])('attr=%i, val=%s → %s', (attribute, attrValue, expected) => {
    const msg = formatCardAttrChanged(game, 1, {
      zoneName: 'table', cardId: 10, attribute, attrValue,
    }, 'Bolt');
    expect(msg).toBe(expected);
  });

  it('returns null for AttrFaceDown (flip path owns that message)', () => {
    const msg = formatCardAttrChanged(game, 1, {
      zoneName: 'table', cardId: 10, attribute: Data.CardAttribute.AttrFaceDown, attrValue: '1',
    }, 'Bolt');
    expect(msg).toBeNull();
  });
});

describe('formatCardCounterChanged', () => {
  const game = gameWithTwoPlayers();
  it('logs added counters with delta', () => {
    const msg = formatCardCounterChanged(game, 1, {
      zoneName: 'table', cardId: 10, counterId: 1, counterValue: 3,
    }, 'Bolt', 1);
    expect(msg).toBe('Alice puts 2 counter(s) on Bolt (total 3).');
  });

  it('logs removed counters with delta', () => {
    const msg = formatCardCounterChanged(game, 1, {
      zoneName: 'table', cardId: 10, counterId: 1, counterValue: 0,
    }, 'Bolt', 3);
    expect(msg).toBe('Alice removes 3 counter(s) from Bolt (total 0).');
  });
});

describe('formatCounterSet', () => {
  const game = gameWithTwoPlayers();
  it('logs increase', () => {
    expect(formatCounterSet(game, 1, { counterId: 1, value: 22 }, 'Life', 20)).toBe('Alice increases their Life to 22.');
  });
  it('logs decrease', () => {
    expect(formatCounterSet(game, 1, { counterId: 1, value: 18 }, 'Life', 20)).toBe('Alice decreases their Life to 18.');
  });
});

describe('formatCardsDrawn', () => {
  const game = gameWithTwoPlayers();
  it('singularizes at 1', () => {
    expect(formatCardsDrawn(game, 1, 1)).toBe('Alice draws a card.');
  });
  it('pluralizes at 2+', () => {
    expect(formatCardsDrawn(game, 1, 7)).toBe('Alice draws 7 cards.');
  });
});

describe('formatZoneShuffled / formatZoneDumped / formatZonePropertiesChanged', () => {
  const game = gameWithTwoPlayers();
  it('shuffle', () => {
    expect(formatZoneShuffled(game, 1)).toBe('Alice shuffles their library.');
  });
  it('dumpZone by owner', () => {
    const msg = formatZoneDumped(game, 1, {
      zoneOwnerId: 1, zoneName: 'deck', numberCards: 4, isReversed: false,
    });
    expect(msg).toBe('Alice looks at 4 card(s) from the top of their library.');
  });
  it('zone alwaysRevealTopCard', () => {
    expect(formatZonePropertiesChanged(game, 1, create(Data.Event_ChangeZonePropertiesSchema, {
      zoneName: 'deck', alwaysRevealTopCard: true, alwaysLookAtTopCard: false,
    }))).toBe('Alice is now revealing the top card of their library.');
  });
});

describe('formatActivePhaseSet / formatActivePlayerSet / formatTurnReversed', () => {
  const game = gameWithTwoPlayers();
  it('phase label matches desktop order', () => {
    expect(formatActivePhaseSet(0)).toBe('It is now the untap step.');
    expect(formatActivePhaseSet(3)).toBe('It is now the first main phase.');
    expect(formatActivePhaseSet(10)).toBe('It is now the end step.');
  });
  it('active player', () => {
    expect(formatActivePlayerSet(game, 2)).toBe('It is now Bob\'s turn.');
  });
  it('turn reversed / restored', () => {
    expect(formatTurnReversed(game, 1, true)).toBe('Alice reverses the turn order.');
    expect(formatTurnReversed(game, 1, false)).toBe('Alice restores the turn order.');
  });
});

describe('formatDieRolled', () => {
  const game = gameWithTwoPlayers();
  it('single-roll', () => {
    expect(formatDieRolled(game, 1, { sides: 20, value: 17, values: [17] })).toBe('Alice rolls a 17 on a 20-sided die.');
  });
  it('multi-roll', () => {
    expect(formatDieRolled(game, 1, { sides: 6, value: 0, values: [3, 4, 5] })).toBe('Alice rolls 3, 4, 5 on 3 6-sided dice.');
  });
  it('no-rolls falls back to bare sides', () => {
    expect(formatDieRolled(game, 1, { sides: 20, value: 0, values: [] })).toBe('Alice rolls a 20-sided die.');
  });
});

describe('formatArrowCreated', () => {
  const game = gameWithTwoPlayers();
  it('card → card arrow', () => {
    const arrow = makeArrow({
      id: 1, startPlayerId: 1, startZone: 'table', startCardId: 10,
      targetPlayerId: 2, targetZone: 'table', targetCardId: 20,
    });
    expect(formatArrowCreated(game, 1, arrow)).toBe('Alice points from Bolt to Bear.');
  });
  it('card → player arrow', () => {
    const arrow = makeArrow({
      id: 1, startPlayerId: 1, startZone: 'table', startCardId: 10,
      targetPlayerId: 2, targetZone: '', targetCardId: -1,
    });
    expect(formatArrowCreated(game, 1, arrow)).toBe('Alice points from Bolt to Bob.');
  });
});

describe('formatPlayerJoined', () => {
  it('logs join', () => {
    const game = gameWithTwoPlayers();
    expect(formatPlayerJoined(game, 2)).toBe('Bob has joined the game.');
  });
});

describe('diffPlayerProperties / formatPropertyDiff', () => {
  const game = gameWithTwoPlayers();
  it('detects concede + ready flips', () => {
    const previous = makePlayerProperties({ playerId: 1, conceded: false, readyStart: false });
    const next = makePlayerProperties({ playerId: 1, conceded: true, readyStart: true });
    const diff = diffPlayerProperties(previous, next);
    expect(diff.conceded).toBe(true);
    expect(diff.ready).toBe(true);
    const msgs = formatPropertyDiff(game, 1, diff);
    expect(msgs).toContain('Alice has conceded the game.');
    expect(msgs).toContain('Alice is ready to start the game.');
  });

  it('detects sideboardLocked / unlocked and deckLoaded', () => {
    const previous = makePlayerProperties({ playerId: 1, sideboardLocked: true, deckHash: 'aaa' });
    const next = makePlayerProperties({ playerId: 1, sideboardLocked: false, deckHash: 'bbb' });
    const diff = diffPlayerProperties(previous, next);
    expect(diff.sideboardUnlocked).toBe(true);
    expect(diff.deckLoaded?.hash).toBe('bbb');
    const msgs = formatPropertyDiff(game, 1, diff);
    expect(msgs).toContain('Alice has unlocked their sideboard.');
    expect(msgs).toContain('Alice has loaded a deck (bbb).');
  });

  it('no diff → no messages', () => {
    const p = makePlayerProperties({ playerId: 1 });
    expect(formatPropertyDiff(game, 1, diffPlayerProperties(p, p))).toHaveLength(0);
  });
});
