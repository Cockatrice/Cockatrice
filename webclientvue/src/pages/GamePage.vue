<template>
  <div class="game">
    <div class="play-area-container">
      <div>
        <!-- This div is a container for the main play area -->
        <div class="play-view-area">
          <!-- Render stacks of cards -->
          <div v-for="(stack, index) in cardStacks" :key="index" class="card-stack">
            <!-- Cards in stack rendering is handled below, this is commented out old version -->
            <!-- <div
          v-for="(card, i) in stack"
          :key="card.id"
          class="card"
          :style="cardStyle(card, i)"
          @mousedown="startDrag(card, $event)"
          @dblclick="toggleTap(card)"
        >
          {{ card.name }}
        </div> -->
          </div>
        </div>

        <div class="play-area">
          <div class="bg-black" ref="playArea" style="height: 100%">
            <div v-for="(stack, index) in cardStacks" :key="index" class="card-stack">
              <div
                v-for="(card, i) in stack"
                :key="card.id"
                class="card"
                :style="cardStyle(card, i)"
                @mousedown="startDrag(card, $event)"
                @dblclick="toggleTap(card)"
                @mouseover="card.peek = true"
                @mouseleave="card.peek = false"
              >
                {{ card.name }}
              </div>
            </div>

            <div
              class="bg-yellow absolute"
              ref="drawArea"
              style="overflow-x: scroll; height: 20%; width: 100%; bottom: 0;"
            ></div>
          </div>

          <!-- For each stack in computed cardStacks, render each card -->
        </div>
      </div>

      <div class="bg-green flex justify-center items-center">
        <!-- Show enlarged "peeked" cards here -->
        <div
          v-for="card in peekedCards"
          :key="card.id"
          class="card"
          :style="{ position: 'relative', width: '200px', height: '260px', zIndex: 100 }"
        >
          <div class="text-center">{{ card.name }}</div>
          <div class="text-center" style="font-size: 12px; color: #666">Peeked</div>
        </div>
      </div>
    </div>

    <div class="controls">
      <button @click="addCard()">Add Card P1</button>
      <button @click="clearBoard">Clear</button>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue';
import type { CSSProperties } from 'vue';

// Define the Card interface, which describes the shape of a card object
interface Card {
  id: number;
  name: string;
  x: number; // x-coordinate on the board grid
  y: number; // y-coordinate on the board grid
  tapped: boolean; // whether the card is tapped (rotated 90 degrees)
  peek?: boolean; // optional: if the card is currently being peeked (hovered)
}

// Constants defining card size in pixels
const CARD_WIDTH = 60;
const CARD_HEIGHT = 90;

// Reactive state holding all cards
const cards = ref<Card[]>([]);

// Currently dragged card (or null if none)
const dragging = ref<Card | null>(null);

// Offset between mouse pointer and card top-left when dragging starts
const offset = ref({ x: 0, y: 0 });

// Unique ID counter for cards
const cardId = ref(1);

// Reference to the play area DOM element (used for boundary calculations)
const playArea = ref<HTMLElement | null>(null);

const drawArea = ref<HTMLElement | null>(null);

// Computed array of cards currently peeked (hovered)
const peekedCards = computed(() => cards.value.filter((c) => c.peek));

// The array of cards being dragged together (stack)
const draggedStack = ref<Card[]>([]);

/**
 * Add a new card at a random but grid-aligned position.
 * The math:
 * - Random x between 100 and 300 px (100 + random*200)
 * - Divided by CARD_WIDTH (60) to snap to grid, rounded
 * - Multiplied back by CARD_WIDTH for exact grid alignment
 * Similar for y, but with CARD_HEIGHT (90).
 */
const addCard = () => {
  if (!drawArea.value || !playArea.value) return;

  const drawAreaRect = drawArea.value.getBoundingClientRect();
  const playAreaRect = playArea.value.getBoundingClientRect();

  // Calculate drawArea's top-left relative to playArea
  const drawAreaLeftRel =
    Math.round((drawAreaRect.left - playAreaRect.left) / CARD_WIDTH) * CARD_WIDTH;
  const drawAreaTopRel =
    Math.round((drawAreaRect.top - playAreaRect.top) / CARD_HEIGHT) * CARD_HEIGHT;

  // Find how many cards already exist horizontally in the drawArea
  const cardsInDrawArea = cards.value.filter(
    (card) =>
      card.y === drawAreaTopRel && // same y line (same row)
      card.x >= drawAreaLeftRel &&
      card.x < drawAreaLeftRel + drawAreaRect.width,
  );

  // Position new card horizontally next to existing cards in drawArea
  const newX = drawAreaLeftRel + cardsInDrawArea.length * CARD_WIDTH;
  const newY = drawAreaTopRel;

  cards.value.push({
    id: cardId.value,
    name: 'Card ' + cardId.value,
    x: newX,
    y: newY,
    tapped: false,
  });

  cardId.value++;
};

/**
 * Clear the board by removing all cards.
 */
const clearBoard = () => {
  cards.value = [];
};

/**
 * Called when user starts dragging a card.
 * - Prevents multiple drag initiations.
 * - Calculates offset between mouse and card position (for smooth drag).
 * - Finds all cards stacked at the same (x, y) to drag the entire stack.
 * - Checks if the dragged card is topmost in that stack.
 *   - If yes, drag whole stack.
 *   - Otherwise, drag only the selected card.
 * - Adds mousemove and mouseup event listeners for dragging.
 */
const startDrag = (card: Card, e: MouseEvent) => {
  if (dragging.value) return;

  dragging.value = card;
  offset.value = {
    x: e.clientX - card.x,
    y: e.clientY - card.y,
  };

  // Get all cards at the same position (stack)
  const stack = cards.value.filter((c) => c.x === card.x && c.y === card.y);

  // The last card in the stack array is the top visually
  const topCard = stack[stack.length - 1];

  if (card.id === topCard?.id) {
    // Drag entire stack if card is top card
    draggedStack.value = stack;
  } else {
    // Drag only the single card otherwise
    draggedStack.value = [card];
  }

  window.addEventListener('mousemove', dragMove);
  window.addEventListener('mouseup', stopDrag);
};

/**
 * Handles the mousemove event during dragging.
 * - Calculates new position relative to play area boundaries.
 * - Snaps the position to the grid by rounding to nearest CARD_WIDTH/HEIGHT multiple.
 * - Ensures cards stay inside play area.
 * - Moves all cards in draggedStack by the calculated delta.
 */
const dragMove = (e: MouseEvent) => {
  if (!dragging.value || !playArea.value) return;

  // Get play area bounding rectangle for constraints
  const rect = playArea.value.getBoundingClientRect();

  // Calculate new X and Y by subtracting the offset
  let newX = e.clientX - offset.value.x;
  let newY = e.clientY - offset.value.y;

  // Snap newX and newY to the grid
  newX = Math.round(newX / CARD_WIDTH) * CARD_WIDTH;
  newY = Math.round(newY / CARD_HEIGHT) * CARD_HEIGHT;

  // Clamp newX and newY so cards do not go out of play area
  newX = Math.max(0, Math.min(newX, rect.width - CARD_WIDTH));
  newY = Math.max(0, Math.min(newY, rect.height - CARD_HEIGHT));

  // Calculate how far the cards moved (delta)
  const dx = newX - dragging.value.x;
  const dy = newY - dragging.value.y;

  // Move all cards in the dragged stack by the delta
  for (const c of draggedStack.value) {
    c.x += dx;
    c.y += dy;
  }
};

/**
 * Called when dragging stops (mouse up).
 * - Removes dragged cards from the current array.
 * - Inserts dragged cards back either at the position of the stack at new location,
 *   or at the end if no stack exists there.
 * - Clears dragging state and removes event listeners.
 */
const stopDrag = () => {
  if (!dragging.value) return;

  const draggedCards = draggedStack.value;

  // Remove dragged cards from original cards array
  for (const card of draggedCards) {
    const index = cards.value.findIndex((c) => c.id === card.id);
    if (index !== -1) {
      cards.value.splice(index, 1);
    }
  }

  // Find if there's an existing stack at the new position
  const insertIndex = cards.value.findIndex(
    (c) => c.x === dragging.value!.x && c.y === dragging.value!.y,
  );

  if (insertIndex === -1) {
    // If no stack exists, add dragged cards at the end
    cards.value.push(...draggedCards);
  } else {
    // Insert dragged cards before the existing stack to keep them visually below
    cards.value.splice(insertIndex, 0, ...draggedCards);
  }

  // Reset drag state
  dragging.value = null;
  draggedStack.value = [];

  // Remove event listeners
  window.removeEventListener('mousemove', dragMove);
  window.removeEventListener('mouseup', stopDrag);
};

/**
 * Toggles the tapped (rotated) state for all cards in the same stack.
 * - When a card is double-clicked, find all cards stacked with it.
 * - If the card is tapped, untap all cards in stack.
 * - Otherwise, tap all cards in stack.
 */
const toggleTap = (card: Card) => {
  // Find all cards at same position
  const sameStack = cards.value.filter((c) => c.x === card.x && c.y === card.y);

  // If tapped, untap; if untapped, tap
  const shouldTap = !card.tapped;

  // Set tapped state for entire stack
  for (const c of sameStack) {
    c.tapped = shouldTap;
  }
};

/**
 * Groups cards into stacks by their (x,y) position.
 * Returns an array of stacks, each stack is an array of cards.
 */
const cardStacks = computed(() => {
  const groups: Record<string, Card[]> = {};
  for (const card of cards.value) {
    // Use x,y as unique key for group
    const key = `${card.x},${card.y}`;
    if (!groups[key]) groups[key] = [];
    groups[key].push(card);
  }
  // Return all groups as array of stacks
  return Object.values(groups);
});

/**
 * Generates CSS styles for each card based on its position and index in stack.
 * - Cards are absolutely positioned based on their x,y grid coordinates.
 * - Each subsequent card in a stack is offset by 12px down and right to create stacking effect.
 * - Tapped cards are rotated 90 degrees.
 * - zIndex increases with stack index to render top cards above bottom ones.
 */
const cardStyle = (card: Card, index: number): CSSProperties => ({
  position: 'absolute',
  top: `${card.y + index * 12}px`, // Vertical position + offset per card in stack
  left: `${card.x + index * 12}px`, // Horizontal position + offset per card in stack
  transform: card.tapped ? 'rotate(90deg)' : 'none', // Rotate if tapped
  transition: 'transform 0.2s ease', // Smooth rotation transition
  zIndex: 10 + index, // Higher index cards appear on top
  backgroundImage: 'url(/mtgCardBack.jpg)', // Background image for card
   backgroundSize: '100% 100%',
});
</script>

<style scoped>
/* Main container for the entire game, flex column fills viewport height */
.game {
  display: flex;
  flex-direction: column;
  height: 100vh;
}

/* Control buttons container */
.controls {
  padding: 10px;
  background: #eee;
  gap: 10px;
}

/* Play area where cards are rendered */
.play-area {
  height: 50%;
  width: 50%;
  background: #4a5568;
  position: absolute;
  bottom: 0;
  overflow: hidden; /* prevent overflow outside play area */
}

/* Upper play view area (red background) */
.play-view-area {
  background: red;
  position: absolute;
  width: 50%;
  height: 50%;
  overflow: hidden;
  top: 0;
}

/* Container for play area and other UI elements */
.play-area-container {
  position: relative;
  display: grid;
  grid-template-columns: 1fr 1fr;
  width: 100%;
  height: 100%;
  overflow: hidden;
}

/* Each card stack container is absolutely positioned */
.card-stack {
  position: absolute;
}

/* Styles for each card */
.card {
  width: 60px; /* matches CARD_WIDTH */
  height: 90px; /* matches CARD_HEIGHT */
  background: #edf2f7;
  border: 1px solid #2d3748;
  border-radius: 4px;
  text-align: center;
  line-height: 90px; /* vertically center card name */
  user-select: none; /* disable text selection */
  cursor: grab;
  font-size: 10px;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2);
}
</style>
