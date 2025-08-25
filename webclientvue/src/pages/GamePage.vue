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

/* Constants defining card size in pixels. The grid is formed in the dragMove function using these constraints so that the xy coordinates
of your mouse are rounded to the nearest multiple of the card width and height relative to the playArea (not viewport). Ex: a mouse location
that is 190px and 130px from the top-left of the playArea will be rounded and place a card to 180px and 90px from the top-left.*/
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

// Context Menu has not been implemented yet, don't worry about it for now
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

// Adds a card after the rightmost card in the player's drawArea
const addCard = () => {
  /* The drawArea and playArea can be null because they were declared as a union variable with type <HTMLElement | null>.
  Thus TypeScript requires that you include a guard to prevent errors*/
  if (!drawArea.value || !playArea.value) return;

  // getBoundingClientRect() returns an object containing playArea’s top, left, right, and bottom coords (measured relative to the viewport)
  const drawAreaRect = drawArea.value.getBoundingClientRect();
  const playAreaRect = playArea.value.getBoundingClientRect();

  /* Getting the drawArea's top-left relative to playArea (instead of viewport) by subtraction, then rounding to the nearest grid value
  (Should be an extremely minor difference, maybe less than 2px)*/
  const drawAreaLeftRel =
    Math.round((drawAreaRect.left - playAreaRect.left) / CARD_WIDTH) * CARD_WIDTH;
  const drawAreaTopRel =
    Math.round((drawAreaRect.top - playAreaRect.top) / CARD_HEIGHT) * CARD_HEIGHT;

  /* Finds the specific cards that exist in the player's drawArea by checking if they’re in the same row as the draw area and if they’re
  within the left and right bounds of the drawArea*/
  const cardsInDrawArea = cards.value.filter(
    (card) =>
      card.y === drawAreaTopRel && // same y line (same row)
      card.x >= drawAreaLeftRel &&
      card.x < drawAreaLeftRel + drawAreaRect.width,
  );

  /* Finds the next x grid value right after the rightmost card. Done by calculating how many cards exist in the drawArea and multiplying it
  by card width.*/
  const newX = drawAreaLeftRel + cardsInDrawArea.length * CARD_WIDTH;
  const newY = drawAreaTopRel;

  // Temporary logic that adds a fake card with an incrementing number ID: (Card 1, Card 2, Card 3...)
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

// Clear the board by removing all cards and resetting cardId
const clearBoard = () => {
  cards.value = [];
  cardId.value = 1;
};

// Called when user starts dragging a card. Tracks dragged card/stack and offset between mouse and card/stack
const startDrag = (card: Card, e: MouseEvent) => {
  // Checks if a drag is already in play to prevent multiple drag initiations.
  if (dragging.value) return;

  /* In short, this works as a boolean in other drag functions. If this value is null, we aren't dragging and the functions return early*/
  dragging.value = card;

  // Later when we add multi-select we will add functionality to allow dragging multiple cards. For now we drag a single card
  draggedStack.value = [card];

  /* Stores offset between mouse and card position so that the card doesn’t end up wildly far away when dragged. We need to fix the offset
  because our mouse position is relative to the viewport, whereas the card position is relative to the  playArea. */
  offset.value = {
    x: e.clientX - card.x,
    y: e.clientY - card.y,
  };

  /* Event listeners are added to the window so that drag events don’t suddenly stop firing when your mouse moves off the card
  in case you move your mouse into a zone the card can't follow it in.*/
  window.addEventListener('mousemove', dragMove);
  window.addEventListener('mouseup', stopDrag);
};

// Handles the mousemove event during dragging. Moves card/stack to where mouse is and snaps to nearest grid location
const dragMove = (e: MouseEvent) => {
  // check if a drag is occurring and if the playArea exists. Return early if not.
  if (!dragging.value || !playArea.value) return;

  // Get an object containing playArea’s top, left, right, and bottom coords (measured relative to the viewport)
  const rect = playArea.value.getBoundingClientRect();

  // Calculate new X and Y by subtracting the offset to keep mouse and card coordinates the same
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

/* Called when dragging stops (mouse up).
 * - Modify cards array so that the dropped cards/card (if in a stack) are placed consecutively in bottom to top order
 * - Unstacks and untaps cards cards in the draw area and auto shifts cards to fill gaps
 * - Clears dragging state and removes event listeners. */
const stopDrag = () => {
  //return early if no card is being dragged or if drawArea and playArea don't exist. Needed to bypass TypeScript warning
  if (!dragging.value || !drawArea.value || !playArea.value) return;

  //tracks which card(s) are being dragged
  const draggedCards = draggedStack.value;

  // getBoundingClientRect() returns an object containing playArea’s top, left, right, and bottom coords (measured relative to the viewport)
  const drawAreaRect = drawArea.value.getBoundingClientRect();
  const playAreaRect = playArea.value.getBoundingClientRect();

  // Getting the drawArea's top-left relative to playArea (instead of viewport) by subtraction, then rounding to the nearest grid value
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

  // If no stack exists, add dragged cards at the end of the cards array (just re-tracking them, no other purpose)
  if (insertIndex === -1) {
    cards.value.push(...draggedCards);
  } else {
    // Insert dragged cards before the existing stack to keep them visually below (this allows us to keep cards in bottom to top order)
    cards.value.splice(insertIndex, 0, ...draggedCards);
  }

  // Find which cards exist in the drawArea (returned cards will not necessarily be in left to right order like we see visually)
  const cardsInDrawArea = cards.value.filter(
    (card) =>
      card.y === drawAreaTopRel && // same y line (same row)
      card.x >= drawAreaLeftRel &&
      card.x < drawAreaLeftRel + drawAreaRect.width,
  );

  // Sort the cards in the draw Area so that this array stores them in left to right, just like we see visually
  cardsInDrawArea.sort((a,b)=>a.x-b.x);

  /* Update all the cards in hand to each be a CARD_WIDTH's length away from the previous card. This is done so that
  a stack of cards added to the hand won't be on top of each other. Each card is also forcibly untapped */
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

// Toggles the tapped (rotated) state for all selected cards. Won't work if the card is in the draw area
const toggleTap = (card: Card) => {

  /* Check if the card exists in the drawArea. The drawArea and playArea can be null because they were declared as a union variable with
  type <HTMLElement | null>. Thus TypeScript requires that you include a guard to prevent errors*/
  if(drawArea.value && playArea.value){

    // getBoundingClientRect() returns an object containing playArea’s top, left, right, and bottom coords (measured relative to the viewport)
    const drawAreaRect = drawArea.value.getBoundingClientRect();
    const playAreaRect = playArea.value.getBoundingClientRect();

    // Getting the drawArea's top-left relative to playArea (instead of viewport) by subtraction, then rounding to the nearest grid value
    const drawAreaTopRel =
      Math.round((drawAreaRect.top - playAreaRect.top) / CARD_HEIGHT) * CARD_HEIGHT;

    // Check if this cards exists in the drawArea and prevent toggling by returning early if so
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

//Groups cards into stacks by their (x,y) position. Returns an array of stacks, each stack is an array of cards.
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

// Need to create stacking logic for everyCard, so that it can be displayed in the red areas
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
