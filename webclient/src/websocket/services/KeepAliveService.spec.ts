import { KeepAliveService } from './KeepAliveService';

import webClient from '../WebClient';

describe('KeepAliveService', () => {
  let service: KeepAliveService;

  beforeEach(() => {
    jest.useFakeTimers();

    service = new KeepAliveService(webClient.socket);
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

      checkReadyStateSpy = jest.spyOn(webClient.socket, 'checkReadyState');
      checkReadyStateSpy.mockImplementation(() => true);

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
      jest.spyOn(service.disconnected$, 'next').mockImplementation(() => {});
      jest.advanceTimersByTime(interval);

      expect(service.disconnected$.next).toHaveBeenCalled();
    });

    it('should endPingLoop if socket is not open', () => {
      jest.spyOn(service, 'endPingLoop').mockImplementation(() => {});
      checkReadyStateSpy.mockImplementation(() => false);

      resolvePing();
      jest.advanceTimersByTime(interval);

      expect(service.endPingLoop).toHaveBeenCalled();
    });
  });
});
