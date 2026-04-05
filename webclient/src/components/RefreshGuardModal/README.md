# RefreshGuard System

The RefreshGuard system prevents users from accidentally losing their game state by refreshing the page or navigating away while in active games or connected to servers.

## Components

### RefreshGuardModal
A Material-UI dialog that warns users about leaving the page.

```tsx
import { RefreshGuardModal } from 'components';

<RefreshGuardModal
  open={true}
  onClose={() => setShowModal(false)}
  onConfirm={() => handleLeave()}
  message="You are in an active game!"
  title="Active Game Warning"
  confirmButtonText="Leave Game"
  cancelButtonText="Stay"
/>
```

### RefreshGuardProvider
Wraps the entire app and automatically shows guard modals based on app state.

```tsx
import { RefreshGuardProvider } from 'components';

<RefreshGuardProvider>
  <YourApp />
</RefreshGuardProvider>
```

## Hooks

### useRefreshGuard
Handles browser refresh and page navigation events.

```tsx
import { useRefreshGuard } from 'hooks';

const { showModal, setShowModal, guardMessage } = useRefreshGuard({
  shouldGuard: isInGame,
  message: 'You are in an active game!',
  onBeforeUnload: () => handleGracefulDisconnect()
});
```

### useNavigationGuard
Handles React Router navigation within the SPA.

```tsx
import { useNavigationGuard } from 'hooks';

const {
  showModal,
  pendingLocation,
  handleConfirmNavigation,
  handleCancelNavigation
} = useNavigationGuard({
  shouldGuard: hasUnsavedChanges,
  message: 'You have unsaved changes!'
});
```

### useGameGuard
Combined hook that handles both refresh and navigation guards based on game state.

```tsx
import { useGameGuard } from 'hooks';

const {
  showRefreshModal,
  showNavigationModal,
  isInActiveGame,
  shouldGuard
} = useGameGuard();
```

## Features

- **Browser beforeunload handling**: Shows native browser dialog
- **Custom modal**: Provides detailed explanation and context
- **SPA navigation blocking**: Prevents React Router navigation
- **Game state awareness**: Automatically enables based on active games
- **Graceful disconnect**: Attempts to leave games cleanly before disconnecting
- **Accessibility**: Proper ARIA labels and keyboard navigation
- **Testing**: Full test coverage for all components and hooks

## Browser Support

- Modern browsers support `beforeunload` events but don't allow custom messages
- Our system shows both the browser's generic dialog AND our custom modal
- The custom modal provides the detailed context that browsers no longer allow

## Implementation Notes

- The system is integrated at the AppShell level for global coverage
- It connects to Redux store to monitor game and connection state
- Graceful disconnect attempts are made before allowing navigation
- Multiple guard conditions can be active simultaneously