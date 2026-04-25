import { useCallback, useId } from 'react';
import {
  useDraggable,
  useDroppable,
  type DraggableAttributes,
  type DraggableSyntheticListeners,
} from '@dnd-kit/core';

import { useScryfallCard } from '@app/hooks';
import { App } from '@app/types';
import type { Data } from '@app/types';

import { makeCardKey, useRegisterCardRef } from '../CardRegistry/CardRegistryContext';

export interface CardSlot {
  smallUrl: string | null | undefined;
  attributes: DraggableAttributes;
  listeners: DraggableSyntheticListeners;
  isDragging: boolean;
  isOver: boolean;
  rootRef: (el: HTMLElement | null) => void;
}

export interface UseCardSlotArgs {
  card: Data.ServerInfo_Card;
  draggable: boolean;
  ownerPlayerId: number | undefined;
  zone: string | undefined;
}

export function useCardSlot({ card, draggable, ownerPlayerId, zone }: UseCardSlotArgs): CardSlot {
  const { smallUrl } = useScryfallCard(card);

  // React-stable id salts the dnd-kit IDs so even two disabled CardSlots
  // rendering the same card (during state transitions / hidden-zone leaks)
  // never collide. Without the salt, pre-owner/zone render cycles shared
  // `card-x-x-<id>` and dnd-kit warned.
  const instanceId = useId();

  const { attributes, listeners, setNodeRef, isDragging } = useDraggable({
    id: `card-${ownerPlayerId ?? instanceId}-${zone ?? 'x'}-${card.id}`,
    data: { card, sourcePlayerId: ownerPlayerId, sourceZone: zone },
    disabled: !draggable || ownerPlayerId == null || zone == null,
  });

  // Cards on the battlefield double as drop targets for drag-to-attach.
  // Other zones don't support attach (desktop's Player::actAttach rejects
  // non-table targets), so the droppable is only live for TABLE. Already-
  // attached cards (children) are also excluded so you can only attach to a
  // lead card — matches the server-side rule that auto-unattaches children
  // when their parent is re-attached.
  //
  // `!isDragging` ensures the currently-dragged card cannot be its own drop
  // target. Without it, the default rectIntersection collision prefers the
  // card's own droppable (smaller, contained in the row) over the battlefield
  // row for short drags — self-drop guard then silently no-ops the move and
  // the UI appears not to update.
  const isAttachedChild = card.attachCardId != null && card.attachCardId !== -1;
  const droppableEnabled =
    ownerPlayerId != null &&
    zone === App.ZoneName.TABLE &&
    !isAttachedChild &&
    !isDragging;
  const { setNodeRef: setDropRef, isOver } = useDroppable({
    id: `card-drop-${ownerPlayerId ?? instanceId}-${zone ?? 'x'}-${card.id}`,
    data: {
      attachTarget: true,
      targetPlayerId: ownerPlayerId,
      targetZone: zone,
      targetCardId: card.id,
    },
    disabled: !droppableEnabled,
  });

  const registryKey =
    ownerPlayerId != null && zone != null
      ? makeCardKey(ownerPlayerId, zone, card.id)
      : null;
  const registerRef = useRegisterCardRef(registryKey);

  const rootRef = useCallback(
    (el: HTMLElement | null) => {
      registerRef(el);
      if (draggable) {
        setNodeRef(el);
      }
      if (droppableEnabled) {
        setDropRef(el);
      }
    },
    [registerRef, setNodeRef, setDropRef, draggable, droppableEnabled],
  );

  return {
    smallUrl,
    attributes,
    listeners,
    isDragging,
    isOver,
    rootRef,
  };
}
