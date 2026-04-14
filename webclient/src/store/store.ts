import { configureStore, isPlain } from '@reduxjs/toolkit';
import { isMessage } from '@bufbuild/protobuf';
import { useDispatch, useSelector } from 'react-redux';
import rootReducer from './rootReducer';

// Protobuf-es v2 messages are already plain objects (no class prototype, unlike v1).
// They carry $typeName (string, identifies the message) and $unknown (binary unknown
// fields) — both are serializable and harmless in Redux state. No conversion needed.
// Fields may include Uint8Array (bytes) and BigInt (int64/uint64), which fail Redux
// Toolkit’s default serializable check, so we extend it to accept these types.
function isSerializable(value: unknown): boolean {
  return isPlain(value) || isMessage(value) || value instanceof Uint8Array || typeof value === 'bigint';
}

export const store = configureStore({
  reducer: rootReducer,
  middleware: (getDefaultMiddleware) =>
    getDefaultMiddleware({
      immutableCheck: { warnAfter: 128 },
      serializableCheck: { isSerializable, warnAfter: 128 },
    }),
});
export type RootState = ReturnType<typeof store.getState>;
export type AppDispatch = typeof store.dispatch;

export const useAppDispatch = useDispatch.withTypes<AppDispatch>();
export const useAppSelector = useSelector.withTypes<RootState>();
