import { useEffect, useState, useCallback } from 'react';
import { useNavigate, useLocation } from 'react-router-dom';

interface UseNavigationGuardOptions {
  shouldGuard: boolean;
  message?: string;
  onBeforeNavigate?: (targetPath: string) => void;
}

interface UseNavigationGuardReturn {
  showModal: boolean;
  setShowModal: (show: boolean) => void;
  pendingLocation: string | null;
  handleConfirmNavigation: () => void;
  handleCancelNavigation: () => void;
  guardMessage: string;
}

/**
 * Hook to guard against navigation within the SPA when user has unsaved changes.
 * Works with React Router to block navigation and show confirmation dialog.
 *
 * @param options Configuration for the navigation guard
 * @returns Modal state controls and navigation handlers
 */
export const useNavigationGuard = ({
  shouldGuard,
  message = 'You have unsaved changes that will be lost.',
  onBeforeNavigate
}: UseNavigationGuardOptions): UseNavigationGuardReturn => {
  const [showModal, setShowModal] = useState(false);
  const [pendingLocation, setPendingLocation] = useState<string | null>(null);

  // Safely get React Router hooks - they might not be available
  let navigate: ReturnType<typeof useNavigate> | null = null;
  let location: ReturnType<typeof useLocation> | null = null;

  try {
    navigate = useNavigate();
    location = useLocation();
  } catch (error) {
    // React Router hooks not available - navigation guard will be disabled
    console.warn('React Router not available, navigation guard disabled:', error);
  }

  const handleConfirmNavigation = useCallback(() => {
    if (pendingLocation && navigate) {
      setShowModal(false);
      setPendingLocation(null);
      navigate(pendingLocation);
    }
  }, [pendingLocation, navigate]);

  const handleCancelNavigation = useCallback(() => {
    setShowModal(false);
    setPendingLocation(null);
  }, []);

  useEffect(() => {
    if (!shouldGuard || !location || !navigate) {
      return; // Skip navigation guard if Router is not available
    }

    const handleLinkClick = (event: Event) => {
      const target = event.target as HTMLElement;
      const link = target.closest('a');

      if (!link) {
        return;
      }

      const href = link.getAttribute('href');
      if (!href || href.startsWith('#') || href.startsWith('http') || href.startsWith('mailto:')) {
        return; // Allow external links, anchors, and mailto links
      }

      // Check if this is a React Router navigation
      if (href !== location.pathname) {
        event.preventDefault();

        if (onBeforeNavigate) {
          onBeforeNavigate(href);
        }

        setPendingLocation(href);
        setShowModal(true);
      }
    };

    const handlePopstate = (event: PopStateEvent) => {
      if (shouldGuard) {
        // Browser back/forward button pressed
        event.preventDefault();

        const targetPath = window.location.pathname;
        if (onBeforeNavigate) {
          onBeforeNavigate(targetPath);
        }

        setPendingLocation(targetPath);
        setShowModal(true);

        // Restore current URL in history
        window.history.pushState(null, '', location.pathname);
      }
    };

    // Add event listeners
    document.addEventListener('click', handleLinkClick);
    window.addEventListener('popstate', handlePopstate);

    return () => {
      document.removeEventListener('click', handleLinkClick);
      window.removeEventListener('popstate', handlePopstate);
    };
  }, [shouldGuard, location?.pathname, onBeforeNavigate, location, navigate]);

  // Auto-hide modal if guard is disabled
  useEffect(() => {
    if (!shouldGuard && showModal) {
      setShowModal(false);
      setPendingLocation(null);
    }
  }, [shouldGuard, showModal]);

  return {
    showModal,
    setShowModal,
    pendingLocation,
    handleConfirmNavigation,
    handleCancelNavigation,
    guardMessage: message
  };
};
