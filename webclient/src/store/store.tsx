import { createStore, applyMiddleware } from 'redux';
import thunk from 'redux-thunk';
import rootReducer from './rootReducer';

const initialState = {};

const middleware: any = [thunk];

export const store = createStore(rootReducer, initialState, applyMiddleware(...middleware));


// console.log('Current State: ', store.getState());

// setInterval(() => {
// 	console.log('Current State: ', store.getState());
// }, 10000)