import { createStore, applyMiddleware } from "redux";
import thunk from "redux-thunk";
import rootReducer from "./rootReducer";

const initialState = {};

const middleware: any = [thunk];

export const store = createStore(rootReducer, initialState, applyMiddleware(...middleware));
