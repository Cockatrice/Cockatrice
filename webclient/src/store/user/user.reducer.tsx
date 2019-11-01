// eslint-disable-next-line
import UserActions from './user.actions';
// eslint-disable-next-line
import UserTypes from './user.types';

const initialState = {
	status: false
};

const userReducer = (state = initialState, action: any) => {
	switch(action.type) {
		default:
			return state;
	}
}

export default userReducer;