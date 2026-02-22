import { useEffect, useState } from 'react';

interface UseRefreshGuardOptions {
  shouldGuard: boolean;
  message?: string;
  onUnload?: () => void;
}

interface UseRefreshGuardReturn {
  showModal: boolean;
  setShowModal: (show: boolean) => void;
  guardMessage: string;
}

/**
 * Hook to guard against accidental page refresh or navigation away from the site.
 * Shows both browser's native beforeunload dialog and a custom modal for better UX.
 *
 * @param options Configuration for the refresh guard
 * @returns Modal state controls and current guard message
 */
export const useRefreshGuard = ({
  shouldGuard,
  message = 'You have unsaved changes that will be lost.',
  onUnload,
}: UseRefreshGuardOptions): UseRefreshGuardReturn => {
  const [showModal, setShowModal] = useState(false);

  useEffect(() => {
    const handleBeforeUnload = (event: BeforeUnloadEvent) => {
      if (!shouldGuard) {
        return;
      }

      // Show browser's native dialog first
      event.preventDefault();
      event.returnValue = '';

      // Schedule custom modal to show after browser dialog is dismissed
      // This only happens if user chooses "Stay" on the browser dialog
      setTimeout(() => {
        if (shouldGuard) {
          setShowModal(true);
        }
      }, 100);

      return '';
    };

    const handleUnload = () => {
      // Only disconnect when page is actually unloading (user chose "Leave")
      if (onUnload) {
        onUnload();
      }
      setShowModal(false);
    };

    if (shouldGuard) {
      window.addEventListener('beforeunload', handleBeforeUnload);
      window.addEventListener('unload', handleUnload);
    }

    return () => {
      window.removeEventListener('beforeunload', handleBeforeUnload);
      window.removeEventListener('unload', handleUnload);
    };
  }, [shouldGuard, message, onUnload]);

  // Auto-hide modal if guard is disabled
  useEffect(() => {
    if (!shouldGuard && showModal) {
      setShowModal(false);
    }
  }, [shouldGuard, showModal]);

  return {
    showModal,
    setShowModal,
    guardMessage: message,
  };
};
