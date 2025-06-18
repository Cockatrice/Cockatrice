<template>
  <q-page class="row items-center justify-evenly q-pa-md" style="height: 90vh; width: 100vw">
    <div class="justify-center" style="height: 90%; width: 65%">
      <q-table
        title="Games"
        :rows="games"
        :columns="gameColumns"
        row-key="name"
        :rows-per-page-options="[0]"
        virtual-scroll
        :pagination-label="(start, end, total) => `Total games: ${total}`"
        style="height: 60%"
        @row-click="
          (evt, row, index) => {
            console.log('Row clicked:', row);
            selectedGame = row;
          }
        "
      />
      <div class="flex justify-end q-mt-md">
        <div class="row q-pa-sm q-gutter-sm">
          <q-btn outline label="Create" color="primary" @click="createGameDialog = true" />
          <!-- @click="redirectGame" -->
          <q-btn outline label="Join" color="secondary" :disable="selectedGame ? false : true" />
          <q-btn outline label="Join as Spectator" :disable="selectedGame ? false : true" />
        </div>
      </div>

      <q-dialog v-model="createGameDialog" persistent>
        <q-card style="min-width: 350px">
          <q-card-section class="row items-center bg-primary">
            <CockatriceAvatar square size="25px" />
            <div class="flex justify-between text-h6" style="color: white; width: 100%">
              <div>Create game</div>

              <q-btn icon="close" flat round dense v-close-popup />
            </div>
          </q-card-section>

          <q-card-section class="row justify-center q-pa-md q-gutter-md">
            <!-- COLUMN 1 -->
            <div class="col q-gutter-sm">
              <q-input
                filled
                label="Description"
                label-color="primary"
                v-model="newGameDesc"
                autofocus
              />

              <q-input
                filled
                label="Players"
                type="number"
                label-color="primary"
                v-model="newGamePlayers"
                autofocus
              />
              <q-select
                filled
                label="Game Type"
                label-color="primary"
                v-model="newGameType"
                :options="gameTypes"
              />
              <div>
                <q-input
                  filled
                  label="Life Total"
                  type="number"
                  label-color="primary"
                  v-model="newGameLifeTotal"
                />
              </div>
            </div>

            <!-- COLUMN 2 -->
            <div class="col q-gutter-sm">
              <q-input
                filled
                label="Password"
                type="password"
                label-color="primary"
                v-model="newGamePassword"
                autofocus
              />
              <q-checkbox label="Only buddies can join" v-model="newGameOnlyBuddies" />
              <q-checkbox label="Only registered users can join" v-model="newGameOnlyRegistered" />
              <q-checkbox label="Spectators can watch" v-model="newGameSpectatorsCanWatch" />
              <q-checkbox
                label="Spectators need a password to watch"
                v-model="newGameSpectatorsNeedPassword"
              />
              <q-checkbox label="Spectators can chat" v-model="newGameSpectatorsCanChat" />
              <q-checkbox label="Spectators can see hands" v-model="newGameSpectatorsCanSeeHands" />
              <q-checkbox label="Create game as Spectator" v-model="newGameAsSpectator" />
            </div>
          </q-card-section>

          <q-card-actions align="right" class="text-primary">
            <q-btn flat label="OK" @click="redirectGame" />
            <q-btn flat label="Clear" />
            <q-btn flat label="CANCEL" @click="createGameDialog = false" />
          </q-card-actions>
        </q-card>
        <!-- END OF LOGIN -->
      </q-dialog>

      <q-card style="height: 30%" class="q-mt-md">
        <q-list bordered separator>
          <q-item clickable v-ripple v-for="chat in chats" :key="chat.timestamp + Math.random()">
            <q-item-section
              style="
                display: flex;
                flex-direction: row;
                justify-content: start;
                align-items: center;
              "
              >{{ chat.timestamp }}
              <q-avatar size=".9em" class="q-ml-sm q-mr-sm"
                ><img :class="chat.avatar === 'user' && 'filter-green'" :src="userBadge"
              /></q-avatar>
              {{ `${chat.username}: ${chat.message}` }}
            </q-item-section>
          </q-item>
        </q-list>
      </q-card>
    </div>
  </q-page>
</template>

<script setup lang="ts">
import userBadge from 'assets/pawn_single.svg';
import { games, gameColumns } from 'src/fake-data/gameColumns';
import type { Games } from 'src/models/models';
import { chats } from 'src/fake-data/chats';
import { useRouter } from 'vue-router';
import { ref } from 'vue';
import gameTypes from 'src/constants/gametypes';

const selectedGame = ref<Games | null>(null);
const createGameDialog = ref(false);

const newGameDesc = ref(''); // Default description for new game
const newGamePlayers = ref(4); // Default to 4 players

//join restrictions
const newGamePassword = ref(''); // Default empty password
const newGameOnlyBuddies = ref(false); // Default to false
const newGameOnlyRegistered = ref(false); // Default to false

//game types
const newGameType = ref(gameTypes[7]); // Default empty game type

//spectators
const newGameSpectatorsCanWatch = ref(false); // Default to false
const newGameSpectatorsNeedPassword = ref(false); // Default to false
const newGameSpectatorsCanChat = ref(false); // Default empty password for spectators
const newGameSpectatorsCanSeeHands = ref(false); // Default to false
const newGameAsSpectator = ref(false); // Default to false

// // Game setup options
const newGameLifeTotal = ref(40); // Default life total

const router = useRouter();

async function redirectGame() {
  console.log('Redirecting to game...');
  await router.push({ name: 'game' });
}
</script>

<style scoped>
.filter-green {
  filter: invert(48%) sepia(79%) saturate(2476%) hue-rotate(86deg) brightness(118%) contrast(119%);
}
</style>
