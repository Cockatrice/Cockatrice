// Server-level session events — server message banner, shutdown schedule,
// user notifications, and connection-closed reason code mapping.

import { create } from '@bufbuild/protobuf';
import { describe, expect, it } from 'vitest';

import { App, Data } from '@app/types';
import { store } from '@app/store';

import { connectAndHandshake } from './helpers/setup';
import {
  buildSessionEventMessage,
  deliverMessage,
} from './helpers/protobuf-builders';

describe('server events', () => {
  it('writes the server banner into server.info.message on Event_ServerMessage', () => {
    connectAndHandshake();

    deliverMessage(buildSessionEventMessage(
      Data.Event_ServerMessage_ext,
      create(Data.Event_ServerMessageSchema, { message: 'Welcome to TestServer!' })
    ));

    expect(store.getState().server.info.message).toBe('Welcome to TestServer!');
  });

  it('stores the shutdown payload on Event_ServerShutdown', () => {
    connectAndHandshake();

    deliverMessage(buildSessionEventMessage(
      Data.Event_ServerShutdown_ext,
      create(Data.Event_ServerShutdownSchema, {
        reason: 'Scheduled maintenance',
        minutes: 5,
      })
    ));

    const shutdown = store.getState().server.serverShutdown;
    expect(shutdown).not.toBeNull();
    expect(shutdown?.reason).toBe('Scheduled maintenance');
    expect(shutdown?.minutes).toBe(5);
  });

  it('appends a notification on Event_NotifyUser', () => {
    connectAndHandshake();

    deliverMessage(buildSessionEventMessage(
      Data.Event_NotifyUser_ext,
      create(Data.Event_NotifyUserSchema, {
        type: Data.Event_NotifyUser_NotificationType.PROMOTION,
        customTitle: 'You have been promoted',
        customContent: 'Now a judge',
      })
    ));

    const notifications = store.getState().server.notifications;
    expect(notifications).toHaveLength(1);
    expect(notifications[0].customTitle).toBe('You have been promoted');
  });

  describe('connection closed', () => {
    it('prefers reasonStr when provided', () => {
      connectAndHandshake();

      deliverMessage(buildSessionEventMessage(
        Data.Event_ConnectionClosed_ext,
        create(Data.Event_ConnectionClosedSchema, {
          reason: Data.Event_ConnectionClosed_CloseReason.OTHER,
          reasonStr: 'kicked by admin',
        })
      ));

      const status = store.getState().server.status;
      expect(status.state).toBe(App.StatusEnum.DISCONNECTED);
      expect(status.description).toBe('kicked by admin');
    });

    it('maps USER_LIMIT_REACHED to a capacity message', () => {
      connectAndHandshake();

      deliverMessage(buildSessionEventMessage(
        Data.Event_ConnectionClosed_ext,
        create(Data.Event_ConnectionClosedSchema, {
          reason: Data.Event_ConnectionClosed_CloseReason.USER_LIMIT_REACHED,
        })
      ));

      expect(store.getState().server.status.description).toContain('maximum user capacity');
    });

    it('maps LOGGEDINELSEWERE to a multi-session message', () => {
      connectAndHandshake();

      deliverMessage(buildSessionEventMessage(
        Data.Event_ConnectionClosed_ext,
        create(Data.Event_ConnectionClosedSchema, {
          reason: Data.Event_ConnectionClosed_CloseReason.LOGGEDINELSEWERE,
        })
      ));

      expect(store.getState().server.status.description).toContain('another location');
    });
  });
});
