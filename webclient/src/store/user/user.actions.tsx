import UserTypes from './user.types';

const UserActions = {
	addUser
};

function addUser() { 
	return function(dispatch: any) {
		dispatch({
			type: UserTypes.ADD_USER,
			payload: { test: 'test' }
		});
	};
}

export default UserActions;