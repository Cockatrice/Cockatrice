import { configureStore, isPlain } from '@reduxjs/toolkit';
import { isMessage } from '@bufbuild/protobuf';
import { useDispatch, useSelector } from 'react-redux';
import rootReducer from './rootReducer';

// Protobuf-es v2 messages are plain objects with $typeName/$unknown siblings;
// bytes fields are Uint8Array and int64/uint64 are BigInt. All four pass through.
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
