<template>
  <div class="game">
    <!-- GRID SYSTEM, 2 columns -->
    <div class="play-area-container" @contextmenu.prevent="openContextMenu($event)">
      <!-- the left side of the screen, the playmats -->
      <div style="display: grid; grid-template-columns: 1fr 1fr; width: 100%; height: 100%">
        <!-- FOR THE OTHER PLAYERS -->
        <div
          v-for="i in numberOfPlayers"
          :key="i"
          :class="[
            'play-view-area',
            i <= Math.ceil(numberOfPlayers / 2) ? 'flipped' : '',
            'bg-red',
          ]"
        >
          <!-- find the total number of players, divide by 2, that is how many draw areas should be flipped, the rest normal, rounded up -->
          <div
            class="draw-view-area bg-blue"
            style="height: 20%; position: absolute; bottom: 0; width: 100%; border-radius: 12px"
          ></div>

          <p class="text-center" style="color: white; font-weight: bold; font-size: 24px">
            Player {{ i }}'s Area
          </p>

          <!--  -->
          <div v-for="(stack, index) in cardStacksOpponent" :key="index">
            <div
              v-for="(card, i) in stack"
              :key="card.id"
              class="card"
              :style="cardStyle(card, i)"
              @mouseover="startPeekTimer(card)"
              @mouseleave="cancelPeekTimer(card)"
            >
              {{ card.name }}
            </div>
          </div>
        </div>

        <!-- <div class="play-view-area flipped">
          <div
            class="draw-view-area bg-yellow"
            style="height: 20%; position: absolute; bottom: 0; width: 100%; border-radius: 12px"
          ></div>
          <div v-for="(stack, index) in cardStacksOpponent" :key="index">
            <div
              v-for="(card, i) in stack"
              :key="card.id"
              class="card"
              :style="cardStyle(card, i)"
              @dblclick="toggleTap(card)"
              @mouseover="startPeekTimer(card)"
              @mouseleave="cancelPeekTimer(card)"
            >
              {{ card.name }}
            </div>
          </div>
        </div> -->

        <!-- FOR OUR PLAYER -->
        <div class="play-area">
          <div class="playAreaStyles" ref="playArea">
            <p
              class="absolute"
              style="
                color: white;
                font-size: 36px;
                text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.8);
                font-weight: bold;
              "
            >
              PLAY AREA
            </p>
            <div v-for="(stack, index) in cardStacks" :key="index" class="card-stack">
              <div
                v-for="(card, i) in stack"
                :key="card.id"
                class="card"
                :style="cardStyle(card, i)"
                @mousedown="startDrag(card, $event)"
                @dblclick="toggleTap(card)"
                @mouseover="startPeekTimer(card)"
                @mouseleave="cancelPeekTimer(card)"
              >
                {{ card.name }}
              </div>
            </div>

            <!-- relative container to make sure absolute draw area stays in bounds -->
            <div class="draw-area" ref="drawArea" style="height: 20%; width: 100%">
              <p
                class="absolute"
                style="
                  font-size: 36px;
                  color: white;
                  text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.8);
                  font-weight: bold;
                "
              >
                DRAW AREA
              </p>
            </div>
          </div>
        </div>
      </div>

      <!-- This is the other stuff area -->
      <div class="peek-area flex justify-center items-center">
        <!-- Show enlarged "peeked" cards here -->
        <div
          v-for="card in peekedCards"
          :key="card.id"
          class="card peeked-card"
          :style="{ position: 'relative', width: '200px', height: '260px', zIndex: 100 }"
        >
          <div class="text-center">{{ card.name }}</div>
          <div class="text-center" style="font-size: 12px; color: #888; margin-top: 8px">
            Peeked
          </div>
        </div>
      </div>
    </div>

    <div class="controls">
      <button @click="addCard()">Add Card P1</button>
      <button @click="clearBoard">Clear</button>
      <button @click="connect()">CONNECT TO SOCKET</button>
    </div>

    <!-- Custom Context Menu -->
    <div
      v-if="contextMenu.visible"
      :style="{ top: `${contextMenu.y - 20}px`, left: `${contextMenu.x - 20}px` }"
      class="context-menu"
      @click.self="contextMenu.visible = false"
      @mouseleave="closeContextMenu"
    >
      <ul>
        <!-- <li @click="action1">Action 1</li>
        <li @click="action2">Action 2</li> -->
      </ul>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, reactive } from 'vue';
import type { CSSProperties } from 'vue';
import socket from '../socket.js'; // Import your socket connection module

// Define the Card interface, which describes the shape of a card object
interface Card {
  id: number;
  name: string;
  x: number; // x-coordinate on the board grid
  y: number; // y-coordinate on the board grid
  tapped: boolean; // whether the card is tapped (rotated 90 degrees)
  peek?: boolean; // optional: if the card is currently being peeked (hovered)
}

interface Player {
  username: string; // Player's username
  cards: Card[]; // Array of cards owned by the player
}

interface ContextMenuState {
  visible: boolean;
  x: number;
  y: number;
  item?: unknown; // Replace `unknown` with your actual item type
}

// Constants defining card size in pixels
const CARD_WIDTH = 60;
const CARD_HEIGHT = 90;

// Reactive state holding all cards
const cards = ref<Card[]>([]);
const everyCard = ref<Player[]>([]); // This will hold all cards, including those not currently displayed

// Currently dragged card (or null if none)
const dragging = ref<Card | null>(null);

// Offset between mouse pointer and card top-left when dragging starts
const offset = ref({ x: 0, y: 0 });

// Unique ID counter for cards
const cardId = ref(1);

// Reference to the play area DOM element (used for boundary calculations)
const playArea = ref<HTMLElement | null>(null);
const drawArea = ref<HTMLElement | null>(null);

const hoverTimers = new Map<number, number>();

const username = ref('Player1'); // Replace with actual username logic
const numberOfPlayers = ref(3); // Replace with actual number of other players logic

// get the cards from updateCards from the server
socket.on('updateCards', (data) => {
  console.log('Received updated cards from server:', data.player.cards);
  // set returned data to everyCard, (each Object will have its own username, and cards) which will then be iterated over to show the cards in the red spaces
  everyCard.value = data.player.cards.map((card: Card) => ({
    username: data.player.username,
    cards: [card],
  }));

  // log the received cards
  console.log('Every card:', everyCard.value);
});
const connect = () => {
  console.log('Connecting to WebSocket server...');

  socket.emit('join', username.value, 'game-room');
  // Here you would typically initialize your WebSocket connection
};

const emitCardsToServer = () => {
  // should emit cards under the current users username
  socket.emit('sync-cards', {
    room: 'game-room',
    // add the current user's username here
    username: username.value, // Replace with actual username logic
    cards: cards.value,
  });
};

const contextMenu = reactive<ContextMenuState>({
  visible: false,
  x: 0,
  y: 0,
});

const openContextMenu = (event: MouseEvent , item?: unknown) => {
  contextMenu.x = event.clientX;
  contextMenu.y = event.clientY;
  contextMenu.item = item;
  contextMenu.visible = true;
};

const closeContextMenu = () => {
  contextMenu.visible = false;
};

// Computed array of cards currently peeked (hovered), should be limited to only one card at a time
const peekedCards = computed(() => {
  return cards.value.filter((card) => card.peek);
});

const startPeekTimer = (card: Card) => {
  cancelPeekTimer(card); // Clear any existing timer first

  const timerId = window.setTimeout(() => {
    for (const c of cards.value) {
      c.peek = c.id === card.id; // Only one card is peeked
    }
  }, 250); // 1 second delay

  hoverTimers.set(card.id, timerId);
};

const cancelPeekTimer = (card: Card) => {
  const timerId = hoverTimers.get(card.id);
  if (timerId !== undefined) {
    clearTimeout(timerId);
    hoverTimers.delete(card.id);
  }
  card.peek = false; // Reset peek on leave
};

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

  emitCardsToServer(); // <-- sync to server
};

//Clear the board by removing all cards and resetting cardId
const clearBoard = () => {
  cards.value = [];
  cardId.value = 1;
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
  // topCard is the last card in the stack (topmost visually)
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
  newX = Math.max(0, Math.min(newX, (Math.round(rect.width/CARD_WIDTH)*CARD_WIDTH) - CARD_WIDTH));
  newY = Math.max(0, Math.min(newY, (Math.round(rect.height/CARD_HEIGHT)*CARD_HEIGHT) - CARD_HEIGHT));

  // Calculate how far the cards moved (delta)
  const dx = newX - dragging.value.x;
  const dy = newY - dragging.value.y;

  // Move all cards in the dragged stack by the delta, this is the actual moving logic, everything else is just setting up the drag
  for (const c of draggedStack.value) {
    c.x += dx;
    c.y += dy;
  }

  // emit updated cards to server
  emitCardsToServer();
};

/**
 * Called when dragging stops (mouse up).
 * - Removes dragged cards from the current array.
 * - Inserts dragged cards back either at the position of the stack at new location,
 *   or at the end if no stack exists there.
 * - Unstacks and untaps cards cards in the draw area and auto shifts cards to fill gaps
 * - Clears dragging state and removes event listeners.
 */
const stopDrag = () => {
  if (!dragging.value || !drawArea.value || !playArea.value) return;
  const draggedCards = draggedStack.value;
  const drawAreaRect = drawArea.value.getBoundingClientRect();
  const playAreaRect = playArea.value.getBoundingClientRect();

  // Calculate drawArea's top-left relative to playArea
  const drawAreaTopRel = Math.round((drawAreaRect.top - playAreaRect.top) / CARD_HEIGHT) * CARD_HEIGHT;
  const drawAreaLeftRel = Math.round((drawAreaRect.left - playAreaRect.left) / CARD_WIDTH) * CARD_WIDTH;


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

  // Find which cards already exist horizontally in the drawArea
  const cardsInDrawArea = cards.value.filter(
    (card) =>
      card.y === drawAreaTopRel && // same y line (same row)
      card.x >= drawAreaLeftRel &&
      card.x < drawAreaLeftRel + drawAreaRect.width,
  );

  // Sort the cards in the draw Area from left to right
  cardsInDrawArea.sort((a,b)=>a.x-b.x);

  //Update the cards to be unstacked seperately in hand and untapped
  cardsInDrawArea.forEach((card, index)=>{
    card.x = drawAreaLeftRel + (index)*CARD_WIDTH
    card.tapped = false;
  });

  // Reset drag state
  dragging.value = null;
  draggedStack.value = [];

  // Remove event listeners
  window.removeEventListener('mousemove', dragMove);
  window.removeEventListener('mouseup', stopDrag);

  //emit updated cards to server
  emitCardsToServer();
};

/**
 * Toggles the tapped (rotated) state for all cards in the same stack.
 * - Won't work if the card is in the draw area
 * - When a card is double-clicked, find all cards stacked with it.
 * - If the card is tapped, untap all cards in stack.
 * - Otherwise, tap all cards in stack.
 */
const toggleTap = (card: Card) => {
  if(drawArea.value && playArea.value){
    const drawAreaRect = drawArea.value.getBoundingClientRect();
    const playAreaRect = playArea.value.getBoundingClientRect();

    // Calculate drawArea's top relative to playArea
    const drawAreaTopRel =
      Math.round((drawAreaRect.top - playAreaRect.top) / CARD_HEIGHT) * CARD_HEIGHT;

    // Check if this cards exists in the drawArea and prevent toggling if so
    if(card.y >= drawAreaTopRel) return;
  }

  // Find all cards at same position
  const sameStack = cards.value.filter((c) => c.x === card.x && c.y === card.y);

  // If tapped, untap; if untapped, tap
  const shouldTap = !card.tapped;

  // Set tapped state for entire stack
  for (const c of sameStack) {
    c.tapped = shouldTap;
  }

  // emit all tapped cards to server
  emitCardsToServer();
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
  // Return all groups as under a single array called cardStacks
  return Object.values(groups);
});

// need to create stacking logic for everyCard, so that it can be displayed in the red areas
// This will be used to display cards in the opponent's areas
const cardStacksOpponent = computed(() => {
  const groups: Record<string, Card[]> = {};
  for (const player of everyCard.value) {
    for (const card of player.cards) {
      // Use x,y as unique key for group
      const key = `${card.x},${card.y}`;
      if (!groups[key]) groups[key] = [];
      groups[key].push(card);
    }
  }
  // Return all groups as under a single array called cardStacks
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
  height: 95vh;
  background: linear-gradient(135deg, #0f0f23 0%, #1a1a2e 50%, #16213e 100%);
}

/* Control buttons container */
.controls {
  padding: 0px;
  background: linear-gradient(135deg, #2c3e50 0%, #3498db 100%);
  gap: 10px;
  border-top: 1px solid rgba(255, 255, 255, 0.1);
  box-shadow: 0 -2px 10px rgba(0, 0, 0, 0.3);
}

.controls button {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  border: none;
  border-radius: 8px;
  padding: 12px 24px;
  font-weight: bold;
  cursor: pointer;
  transition: all 0.3s ease;
  box-shadow: 0 4px 15px rgba(0, 0, 0, 0.2);
}

.controls button:hover {
  transform: translateY(-2px);
  box-shadow: 0 6px 20px rgba(0, 0, 0, 0.3);
  background: linear-gradient(135deg, #764ba2 0%, #667eea 100%);
}

.controls button:active {
  transform: translateY(0);
  box-shadow: 0 2px 10px rgba(0, 0, 0, 0.2);
}

/* Play area where cards are rendered */
.play-area {
  height: 100%;
  width: 100%;
  position: relative;
}

/* Upper play view area (opponent areas) */
.play-view-area {
  background: linear-gradient(135deg, #ff6b6b 0%, #ee5a24 50%, #c0392b 100%);
  border: 2px solid rgba(255, 255, 255, 0.2);
  border-radius: 12px;
  color: white;
  position: relative;
  text-align: center;
  display: flex;
  justify-content: center;
  align-items: center;
  width: 100%;
  height: 100%;
  top: 0;
  box-shadow: inset 0 2px 10px rgba(0, 0, 0, 0.3);
  font-weight: bold;
  text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.5);
}

/* Container for play area and other UI elements */
.play-area-container {
  position: relative;
  display: grid;
  grid-template-columns: 4fr 1fr;
  width: 100%;
  height: 100%;
  gap: 8px;
}

.playAreaStyles {
  height: 100%;
  display: flex;
  flex-grow: 1;
  flex-direction: column;
  position: relative;
  border: 2px solid rgba(255, 255, 255, 0.2);
  background: linear-gradient(135deg, #2ecc71 0%, #27ae60 50%, #229954 100%);
  border-radius: 12px;
  box-shadow: inset 0 2px 10px rgba(0, 0, 0, 0.3);
  overflow-y: hidden;
}

.draw-area {
  margin-top: auto;
  background: linear-gradient(135deg, #3498db 0%, #2980b9 50%, #1f4e79 100%);
  border-radius: 12px;
  border: 2px solid rgba(255, 255, 255, 0.2);
  box-shadow: inset 0 2px 10px rgba(0, 0, 0, 0.3);
  height: 20%;
  width: 100%;
  overflow-x: auto; /* Allow horizontal scrolling */
  overflow-y: hidden;
  white-space: nowrap; /* Prevent line breaks so cards line up horizontally */
  position: relative;
  padding: 10px;
  box-sizing: border-box;
}

.peek-area {
  background: linear-gradient(135deg, #9b59b6 0%, #8e44ad 50%, #663399 100%);
  border: 2px solid rgba(255, 255, 255, 0.2);
  border-radius: 12px;
  box-shadow: inset 0 2px 10px rgba(0, 0, 0, 0.3);
}

/* Each card stack container is absolutely positioned */
.card-stack {
  position: absolute;
}

/* Styles for each card */
.card {
  width: 60px; /* matches CARD_WIDTH */
  height: 90px; /* matches CARD_HEIGHT */
  background: linear-gradient(135deg, #f8f9fa 0%, #e9ecef 50%, #dee2e6 100%);
  border: 2px solid #495057;
  border-radius: 8px;
  text-align: center;
  line-height: 90px; /* vertically center card name */
  user-select: none; /* disable text selection */
  cursor: grab;
  font-size: 10px;
  font-weight: bold;
  color: #212529;
  box-shadow: 0 4px 15px rgba(0, 0, 0, 0.3);
  transition: all 0.2s ease;
}

.card:hover {
  transform: translateY(-2px);
  box-shadow: 0 6px 20px rgba(0, 0, 0, 0.4);
  border-color: #6c757d;
}

.peeked-card {
  background: linear-gradient(135deg, #fff3cd 0%, #ffeaa7 50%, #fdcb6e 100%);
  border: 3px solid #e17055;
  box-shadow: 0 8px 25px rgba(0, 0, 0, 0.4);
  transform: scale(1.05);
}

.flipped {
  transform: scaleY(-1) scaleX(-1); /* Flip the card */
}

.context-menu {
  position: absolute;
  height: 200px;
  width: 150px;
  background: rgba(255, 255, 255, 0.9);
  border: 1px solid #ccc;
  border-radius: 8px;
  padding: 10px;
  z-index: 100;
  box-shadow: 0 4px 15px rgba(0, 0, 0, 0.2);
}
</style>
