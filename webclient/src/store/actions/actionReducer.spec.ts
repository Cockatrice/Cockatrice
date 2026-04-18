import { actionReducer } from './actionReducer';

describe('actionReducer', () => {
  it('stores the init action type and starts count at 1', () => {
    const result = actionReducer(undefined, { type: '@@INIT' });
    expect(result.type).toBe('@@INIT');
    expect(result.payload).toBeNull();
    expect(result.meta).toBeNull();
    expect(result.error).toBe(false);
    expect(result.count).toBe(1);
  });

  it('stores action type and cloned payload', () => {
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

  it('replaces type from previous action', () => {
    const initial = actionReducer(undefined, { type: 'FIRST', payload: 'original' });
    const result = actionReducer(initial, { type: 'SECOND' });
    expect(result.type).toBe('SECOND');
    expect(result.payload).toBeNull();
    expect(result.count).toBe(2);
  });

  it('stores meta and error from action', () => {
    const result = actionReducer(undefined, { type: 'ERR', meta: { source: 'api' }, error: true });
    expect(result.meta).toEqual({ source: 'api' });
    expect(result.error).toBe(true);
  });

  it('deep-clones payload to prevent shared references', () => {
    const nested = { data: { counterInfo: { id: 1, count: 20 } } };
    const result = actionReducer(undefined, { type: 'TEST', payload: nested });
    expect(result.payload).toEqual(nested);
    expect(result.payload).not.toBe(nested);
    expect((result.payload as any).data).not.toBe(nested.data);
    expect((result.payload as any).data.counterInfo).not.toBe(nested.data.counterInfo);
  });

  it('deep-clones meta to prevent shared references', () => {
    const meta = { source: { nested: true } };
    const result = actionReducer(undefined, { type: 'TEST', meta });
    expect(result.meta).toEqual(meta);
    expect(result.meta).not.toBe(meta);
  });
});
