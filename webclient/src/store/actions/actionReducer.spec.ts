import { actionReducer } from './actionReducer';

describe('actionReducer', () => {
  it('spreads the init action onto state and starts count at 1', () => {
    const result = actionReducer(undefined, { type: '@@INIT' });
    // actionReducer always spreads the action, so type reflects the dispatched action
    expect(result.type).toBe('@@INIT');
    expect(result.payload).toBeNull();
    expect(result.meta).toBeNull();
    expect(result.error).toBe(false);
    expect(result.count).toBe(1);
  });

  it('spreads action onto state and increments count', () => {
    const result = actionReducer(undefined, { type: 'MY_ACTION', payload: { id: 1 } });
    expect(result.type).toBe('MY_ACTION');
    expect(result.payload).toEqual({ id: 1 });
    expect(result.count).toBe(1);
  });

  it('increments count on each dispatch', () => {
    const state1 = actionReducer(undefined, { type: 'A' });
    const state2 = actionReducer(state1, { type: 'B' });
    const state3 = actionReducer(state2, { type: 'C' });
    expect(state3.count).toBe(3);
  });

  it('preserves existing state fields not overridden by action', () => {
    const initial = actionReducer(undefined, { type: 'FIRST', payload: 'original' });
    const result = actionReducer(initial, { type: 'SECOND' });
    expect(result.type).toBe('SECOND');
    expect(result.count).toBe(2);
  });

  it('spreads action.meta and action.error from action onto state', () => {
    const result = actionReducer(undefined, { type: 'ERR', meta: { source: 'api' }, error: true });
    expect(result.meta).toEqual({ source: 'api' });
    expect(result.error).toBe(true);
  });
});
