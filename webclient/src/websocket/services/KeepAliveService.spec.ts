vi.mock('../WebClient', () => ({
  __esModule: true,
  default: {
    socket: {
      checkReadyState: vi.fn(),
    },
  },
}));

import { KeepAliveService } from './KeepAliveService';

import webClient from '../WebClient';

describe('KeepAliveService', () => {
  let service: KeepAliveService;

  beforeEach(() => {
    vi.useFakeTimers();

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

      checkReadyStateSpy = vi.spyOn(webClient.socket, 'checkReadyState');
      checkReadyStateSpy.mockImplementation(() => true);

      service.startPingLoop(interval, ping);
      vi.advanceTimersByTime(interval);
    });

    it('should start ping loop', () => {
      expect((service as any).keepalivecb).toBeDefined();
      expect((service as any).lastPingPending).toBeTruthy();
    });

    it('should call ping callback when done', () => {
      resolvePing();

      return promise.then(() => {
        expect((service as any).lastPingPending).toBeFalsy();
      });
    });

    it('should fire disconnected$ if lastPingPending is still true', () => {
      vi.spyOn(service.disconnected$, 'next').mockImplementation(() => {});
      vi.advanceTimersByTime(interval);

      expect(service.disconnected$.next).toHaveBeenCalled();
    });

    it('should endPingLoop if socket is not open', () => {
      vi.spyOn(service, 'endPingLoop').mockImplementation(() => {});
      checkReadyStateSpy.mockImplementation(() => false);

      resolvePing();
      vi.advanceTimersByTime(interval);

      expect(service.endPingLoop).toHaveBeenCalled();
    });
  });
});
