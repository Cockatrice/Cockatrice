import React from 'react';
import { Link } from "react-router-dom";

import { RouteEnum } from '../../common/types';

export function Navigation(props) {
	return (
		<nav className="Header-nav">
			<ul className="Header-nav__items">
				<li className="Header-nav__item">
					<Link to={RouteEnum.DECKS}>
						Decks
					</Link>
				</li>
				<strong>|</strong>
				<Link to={RouteEnum.ACCOUNT}>
					<div className="Header-account">
						<span className="Header-account__name">
							{props.user.name}
						</span>
						<span className="Header-account__indicator"></span>
					</div>
				</Link>
			</ul>
		</nav>
	);
}