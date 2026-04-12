import { CommonEvents } from './index';

describe('CommonEvents', () => {
  it('is an empty event map (all common events were moved to game/session events)', () => {
    expect(CommonEvents).toEqual({});
  });
});
