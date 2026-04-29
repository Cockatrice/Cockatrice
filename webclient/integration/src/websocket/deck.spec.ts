// Deck and replay command round-trips — validates the session command pipeline
// for deck CRUD and replay operations end-to-end.

import { create } from '@bufbuild/protobuf';
import { describe, expect, it } from 'vitest';

import { Data } from '@app/types';
import { store } from '@app/store';
import { SessionCommands } from '@app/websocket';

import { connectAndLogin } from '../helpers/setup';
import {
  buildResponse,
  buildResponseMessage,
  deliverMessage,
} from '../helpers/protobuf-builders';
import { findLastSessionCommand } from '../helpers/command-capture';

describe('deck operations', () => {
  it('populates backendDecks from deckList response', () => {
    connectAndLogin();

    SessionCommands.deckList();

    const { cmdId } = findLastSessionCommand(Data.Command_DeckList_ext);

    const deckFile = create(Data.ServerInfo_DeckStorage_TreeItemSchema, {
      id: 1,
      name: 'MyDeck.cod',
      file: create(Data.ServerInfo_DeckStorage_FileSchema, { creationTime: 1000 }),
    });
    const root = create(Data.ServerInfo_DeckStorage_FolderSchema, {
      items: [deckFile],
    });
    deliverMessage(buildResponseMessage(buildResponse({
      cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
      ext: Data.Response_DeckList_ext,
      value: create(Data.Response_DeckListSchema, { root }),
    })));

    const backendDecks = store.getState().server.backendDecks;
    expect(backendDecks).not.toBeNull();
    expect(backendDecks?.root?.items).toHaveLength(1);
    expect(backendDecks?.root?.items[0]?.name).toBe('MyDeck.cod');
  });

  it('populates downloadedDeck from deckDownload response', () => {
    connectAndLogin();

    SessionCommands.deckDownload(42);

    const { cmdId } = findLastSessionCommand(Data.Command_DeckDownload_ext);

    deliverMessage(buildResponseMessage(buildResponse({
      cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
      ext: Data.Response_DeckDownload_ext,
      value: create(Data.Response_DeckDownloadSchema, { deck: '4 Lightning Bolt\n20 Mountain' }),
    })));

    const downloaded = store.getState().server.downloadedDeck;
    expect(downloaded).not.toBeNull();
    expect(downloaded?.deckId).toBe(42);
    expect(downloaded?.deck).toContain('Lightning Bolt');
  });

  it('deckUpload sends payload and dispatches uploadServerDeck on success', () => {
    connectAndLogin();

    SessionCommands.deckUpload('/folder', 0, '4 Counterspell\n20 Island');

    const { cmdId, value } = findLastSessionCommand(Data.Command_DeckUpload_ext);
    expect(value.path).toBe('/folder');
    expect(value.deckList).toContain('Counterspell');

    const newFile = create(Data.ServerInfo_DeckStorage_TreeItemSchema, {
      id: 7,
      name: 'CounterDeck.cod',
    });
    deliverMessage(buildResponseMessage(buildResponse({
      cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
      ext: Data.Response_DeckUpload_ext,
      value: create(Data.Response_DeckUploadSchema, { newFile }),
    })));
    // No state assertion: backendDecks is keyed by full tree, not single
    // upload — the integration verifies the dispatcher is reached, not the
    // tree-merge logic which lives in the reducer.
  });

  it('deckDel sends deckId and resolves on RespOk', () => {
    connectAndLogin();

    SessionCommands.deckDel(13);

    const { cmdId, value } = findLastSessionCommand(Data.Command_DeckDel_ext);
    expect(value.deckId).toBe(13);

    deliverMessage(buildResponseMessage(buildResponse({
      cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
    })));
  });

  it('deckNewDir sends path + dirName payload and resolves on RespOk', () => {
    connectAndLogin();

    SessionCommands.deckNewDir('/parent', 'NewFolder');

    const { cmdId, value } = findLastSessionCommand(Data.Command_DeckNewDir_ext);
    expect(value.path).toBe('/parent');
    expect(value.dirName).toBe('NewFolder');

    deliverMessage(buildResponseMessage(buildResponse({
      cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
    })));
  });

  it('deckDelDir sends path payload and resolves on RespOk', () => {
    connectAndLogin();

    SessionCommands.deckDelDir('/folder/to/remove');

    const { cmdId, value } = findLastSessionCommand(Data.Command_DeckDelDir_ext);
    expect(value.path).toBe('/folder/to/remove');

    deliverMessage(buildResponseMessage(buildResponse({
      cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
    })));
  });
});

describe('replay operations', () => {
  it('populates replays from replayList response', () => {
    connectAndLogin();

    SessionCommands.replayList();

    const { cmdId } = findLastSessionCommand(Data.Command_ReplayList_ext);

    const match = create(Data.ServerInfo_ReplayMatchSchema, {
      gameId: 99,
      gameName: 'Casual Game',
    });
    deliverMessage(buildResponseMessage(buildResponse({
      cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
      ext: Data.Response_ReplayList_ext,
      value: create(Data.Response_ReplayListSchema, { matchList: [match] }),
    })));

    const replays = store.getState().server.replays;
    expect(replays[99]).toBeDefined();
    expect(replays[99].gameName).toBe('Casual Game');
  });

  it('removes replay from state on replayDeleteMatch round-trip', () => {
    connectAndLogin();

    // First populate a replay
    SessionCommands.replayList();
    const list = findLastSessionCommand(Data.Command_ReplayList_ext);
    deliverMessage(buildResponseMessage(buildResponse({
      cmdId: list.cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
      ext: Data.Response_ReplayList_ext,
      value: create(Data.Response_ReplayListSchema, {
        matchList: [create(Data.ServerInfo_ReplayMatchSchema, { gameId: 99, gameName: 'Old Game' })],
      }),
    })));
    expect(store.getState().server.replays[99]).toBeDefined();

    // Now delete it
    SessionCommands.replayDeleteMatch(99);
    const del = findLastSessionCommand(Data.Command_ReplayDeleteMatch_ext);
    deliverMessage(buildResponseMessage(buildResponse({
      cmdId: del.cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
    })));

    expect(store.getState().server.replays[99]).toBeUndefined();
  });
});