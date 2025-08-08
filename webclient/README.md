## Application Architecture

![Application Architecture](architecture.png?raw=true "Application Architecture")

This project was bootstrapped with [Create React App](https://github.com/facebook/create-react-app).

## Local Development with Docker

To develop and test the webclient against a local Servatrice server:

### Prerequisites

- Docker and Docker Compose
- Node.js and npm

### Setup Servatrice Server

1. Navigate to the Cockatrice root directory
2. Start the Servatrice server with Docker:
   ```bash
   docker compose up -d
   ```
3. The server will be available at `ws://localhost:4748`
4. Set authentication to disabled in `servatrice/docker/servatrice-docker.ini`
   ```
   [authentication]
   method=none
   ```

### Setup Webclient

1. Navigate to the `webclient` directory
2. Install dependencies:
   ```bash
   npm install
   ```
3. Start the development server:
   ```bash
   npm start
   ```
4. Open [http://localhost:3000](http://localhost:3000)
5. The "Local Server" option should be available in the server list
6. Use any username to connect (no password required)

### Troubleshooting

- Ensure Docker container exposes port 4748: `docker ps` should show `0.0.0.0:4748->4748/tcp`
- Check Servatrice logs: `docker compose logs servatrice`
- Verify WebSocket connection in browser developer tools (Network tab)

## Available Scripts

In the project directory, you can run:

### `npm start`

Runs the app in the development mode.<br />
Open [http://localhost:3000](http://localhost:3000) to view it in the browser.

The page will reload if you make edits.<br />
You will also see any lint errors in the console.

### `npm test`

Launches the test runner in the interactive watch mode.<br />
See the section about [running tests](https://facebook.github.io/create-react-app/docs/running-tests) for more information.

### `npm run build`

Builds the app for production to the `build` folder.<br />
It correctly bundles React in production mode and optimizes the build for the best performance.

The build is minified and the filenames include the hashes.<br />
Your app is ready to be deployed!

See the section about [deployment](https://facebook.github.io/create-react-app/docs/deployment) for more information.

### `npm run eject`

**Note: this is a one-way operation. Once you `eject`, you can’t go back!**

If you aren’t satisfied with the build tool and configuration choices, you can `eject` at any time. This command will remove the single build dependency from your project.

Instead, it will copy all the configuration files and the transitive dependencies (Webpack, Babel, ESLint, etc) right into your project so you have full control over them. All of the commands except `eject` will still work, but they will point to the copied scripts so you can tweak them. At this point you’re on your own.

You don’t have to ever use `eject`. The curated feature set is suitable for small and middle deployments, and you shouldn’t feel obligated to use this feature. However we understand that this tool wouldn’t be useful if you couldn’t customize it when you are ready for it.

## Learn More

You can learn more in the [Create React App documentation](https://facebook.github.io/create-react-app/docs/getting-started).

To learn React, check out the [React documentation](https://reactjs.org/).

## To-Do List

1. ✔️ RefreshGuard modal

2. Disable AutoScrollToBottom when the user has scrolled up

- when the user scrolls back to bottom, it should renable
- renable after a period of inactivity (3 minutes?)

3. Figure out how to type components w/ RouteComponentProps

- Component<RouteComponentProps<???, ???, ???>>

4. clear input onSubmit

5. figure out how to reflect server status changes in the ui

6. Account page

7. Register/Reset Password forms

8. Message User

9. Main Nav scheme
