import { KeepAliveService } from './KeepAliveService';
import { WebSocketService } from "./WebSocketService";

import webClient from '../WebClient';

describe('KeepAliveService', () => {
  let service: KeepAliveService;
  let socket: WebSocketService;

  beforeEach(() => {
    jest.useFakeTimers();

    socket = new WebSocketService(webClient);
    service = new KeepAliveService(socket);
  });

  it('should create', () => {
    expect(service).toBeDefined();
  });

  describe('startPingLoop', () => {
    let resolvePing;
    let interval;
    let promise;
    let ping;
    let checkReadyStateSpy;

    beforeEach(() => {
      interval = 100;
      promise = new Promise(resolve => resolvePing = resolve);
      ping = (done) => promise.then(done);

      checkReadyStateSpy = spyOn(socket, 'checkReadyState');
      checkReadyStateSpy.and.returnValue(true);

      service.startPingLoop(interval, ping);
      jest.advanceTimersByTime(interval);
    });

    it('should start ping loop', () => {
      expect((service as any).keepalivecb).toBeDefined();
      expect((service as any).lastPingPending).toBeTruthy();
    });

    it('should call ping callback when done', (done: jest.DoneCallback) => {
      resolvePing();

      promise.then(() => {
        expect((service as any).lastPingPending).toBeFalsy();
        done();
      });
    });

    it('should fire disconnected$ if lastPingPending is still true', () => {
      spyOn(service.disconnected$, 'next');
      jest.advanceTimersByTime(interval);

      expect(service.disconnected$.next).toHaveBeenCalled();
    });

    it('should endPingLoop if socket is not open', () => {
      spyOn(service, 'endPingLoop');
      checkReadyStateSpy.and.returnValue(false);

      resolvePing();
      jest.advanceTimersByTime(interval);

      expect(service.endPingLoop).toHaveBeenCalled();
    });
  });
});
