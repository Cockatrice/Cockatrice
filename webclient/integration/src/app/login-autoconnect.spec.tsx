// Full-stack autoconnect integration. Only outbound surfaces are mocked
// (WebSocket via the shared setup, IndexedDB via fake-indexeddb in setup).
// Everything in between — Dexie, DTOs, useSettings/useKnownHosts, useAutoLogin,
// the Login container, WebClient, Redux — runs as shipped code.
//
// We assert auto-login via `connectionAttemptMade` on the real server slice,
// not via the WebSocket mock's call count: KnownHosts fires a testConnection
// on mount for the UX indicator, which also constructs sockets, so raw
// socket counts are noisy. Only the login path dispatches CONNECTION_ATTEMPTED.

import { act, waitFor } from '@testing-library/react';
import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';

// Store loads notify React subscribers synchronously when the Dexie
// promise resolves. Awaiting whenReady() directly would let those
// notifications fire outside an act() scope, which trips React's
// "update was not wrapped in act" warning. Wrapping here captures
// both the store resolution and any resulting component re-renders.
const flushStoresAndEffects = async (): Promise<void> => {
  await act(async () => {
    await settingsStore.whenReady();
    await knownHostsStore.whenReady();
    // Let dependent effects (host-sync, settings-sync) commit.
    await new Promise((resolve) => setTimeout(resolve, 0));
  });
};

import { autoLoginGate } from '../../../src/hooks/useAutoLogin';
import { settingsStore } from '../../../src/hooks/useSettings';
import { knownHostsStore } from '../../../src/hooks/useKnownHosts';
import Login from '../../../src/containers/Login/Login';
import { HostDTO, SettingDTO } from '@app/services';
import { App } from '@app/types';
import { ServerSelectors, ServerDispatch } from '@app/store';
import { WebsocketTypes } from '@app/websocket/types';

import { resetDexie } from '../services/dexie/resetDexie';
import { renderAppScreen, store } from './helpers';

// Mimics the production "user logged out / connection dropped" transition:
// dispatching updateStatus(DISCONNECTED) is what the real reducer uses to
// clear connectionAttemptMade (clearStore intentionally preserves status).
const simulateLogout = () => {
  ServerDispatch.updateStatus(WebsocketTypes.StatusEnum.DISCONNECTED, null);
};

const seedAutoConnect = async () => {
  const setting = new SettingDTO(App.APP_USER);
  setting.autoConnect = true;
  await setting.save();

  const id = (await HostDTO.add({
    name: 'Test Server',
    host: 'server.example',
    port: '4748',
    editable: false,
  })) as number;
  const host = (await HostDTO.get(id))!;
  host.remember = true;
  host.userName = 'alice';
  host.hashedPassword = 'stored-hash';
  host.lastSelected = true;
  await host.save();
};

const attempted = (): boolean =>
  ServerSelectors.getConnectionAttemptMade(store.getState());

afterEach(async () => {
  // Absorb any state updates that lingered past the test body (stores
  // resolving after unmount, trailing effect commits) so they're wrapped
  // in act and don't trip React's warning during teardown.
  await flushStoresAndEffects();
});

beforeEach(async () => {
  // setup.ts's beforeEach installs fake timers and re-creates the WebClient
  // singleton. Dexie + React async need real timers; module caches persist
  // across tests and need explicit reset.
  vi.useRealTimers();
  await resetDexie();

  // Reset the module-level caches that load from Dexie. Without this, a
  // test would read the PREVIOUS test's snapshot (the Dexie clear only
  // truncates storage, not the useSettings / useKnownHosts subscribers'
  // cached values).
  settingsStore.reset();
  knownHostsStore.reset();
  autoLoginGate.hasChecked = false;
});

describe('autoconnect — cold start', () => {
  it('auto-logs in when Dexie has autoConnect=true + host with stored credentials', async () => {
    await seedAutoConnect();

    renderAppScreen(<Login />);

    await waitFor(() => {
      expect(attempted()).toBe(true);
    });
  });

  it('does NOT attempt login when Dexie has no settings row', async () => {
    renderAppScreen(<Login />);

    await flushStoresAndEffects();

    expect(attempted()).toBe(false);
  });

  it('does NOT attempt login when autoConnect=true but lastSelected host lacks credentials', async () => {
    const setting = new SettingDTO(App.APP_USER);
    setting.autoConnect = true;
    await setting.save();
    await HostDTO.add({
      name: 'Unremembered',
      host: 'server.example',
      port: '4748',
      editable: false,
      lastSelected: true,
    });

    renderAppScreen(<Login />);

    await flushStoresAndEffects();

    expect(attempted()).toBe(false);
  });
});

describe('autoconnect — logout cycle (same session)', () => {
  it('does not auto-reconnect after first auto-login + logout within the same JS session', async () => {
    await seedAutoConnect();

    const first = renderAppScreen(<Login />);
    await waitFor(() => {
      expect(attempted()).toBe(true);
    });

    // Simulate "logged out and returned to /login": unmount, clear the
    // store's connectionAttemptMade flag (the app-level equivalent of
    // DISCONNECTED → status.connectionAttemptMade reset), remount.
    first.unmount();
    simulateLogout();

    renderAppScreen(<Login />);
    await flushStoresAndEffects();

    // The session gate must have kept useAutoLogin silent; the flag stays
    // false.
    expect(attempted()).toBe(false);
  });

  it('does not auto-connect when the user enabled autoConnect mid-session and then logged out', async () => {
    // First mount with autoConnect=false — gate latches without firing.
    const first = renderAppScreen(<Login />);
    await flushStoresAndEffects();
    expect(attempted()).toBe(false);
    first.unmount();

    // Mid-session: user ticked the checkbox → Dexie flipped to autoConnect=true.
    await seedAutoConnect();

    // Remount (post-logout). The gate MUST keep useAutoLogin silent.
    renderAppScreen(<Login />);
    await flushStoresAndEffects();

    expect(attempted()).toBe(false);
  });
});

describe('autoconnect — refresh', () => {
  it('auto-connects again after resetting the session gate (page-refresh equivalent)', async () => {
    await seedAutoConnect();

    const first = renderAppScreen(<Login />);
    await waitFor(() => {
      expect(attempted()).toBe(true);
    });
    first.unmount();

    // Simulate a browser refresh: the session gate naturally resets on a
    // fresh JS context, and the real connection flag resets too.
    simulateLogout();
    autoLoginGate.hasChecked = false;

    renderAppScreen(<Login />);
    await waitFor(() => {
      expect(attempted()).toBe(true);
    });
  });
});
