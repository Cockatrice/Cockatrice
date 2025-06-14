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
      />

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
import { chats } from 'src/fake-data/chats';
</script>

<style scoped>
.filter-green {
  filter: invert(48%) sepia(79%) saturate(2476%) hue-rotate(86deg) brightness(118%) contrast(119%);
}
</style>
