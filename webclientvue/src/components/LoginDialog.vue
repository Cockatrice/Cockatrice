<template>
  <q-dialog v-model="loginDialog" persistent>
    <!-- LOGIN -->
    <q-card style="min-width: 350px" v-if="!signUp">
      <q-card-section class="row items-center q-gutter-md bg-primary">
        <CockatriceAvatar square size="25px" />
        <div class="text-h6" style="color: white">Login to Webatrice</div>
      </q-card-section>

      <q-card-section class="q-pt-none column q-pa-lg q-gutter-lg">
        <q-select
          filled
          v-model="serverDropdown"
          :options="serverOptions"
          label="Select Server"
          label-color="primary"
        />
        <q-input label-color="primary" v-model="username" label="Username" autofocus />
        <q-input
          v-model="password"
          label="Password"
          type="password"
          label-color="primary"
          @keyup.enter="loginDialog = false"
        />
        <q-card-section class="row items-center">
          <q-checkbox v-model="savePassword" label="Save password?" class="q-mr-lg" />
          <a href="#">Forgot Password</a>
        </q-card-section>
      </q-card-section>

      <q-card-actions align="right" class="text-primary">
        <q-btn flat label="Sign Up" @click="signUp = true" />
        <q-btn flat label="Login" @click="login" />
      </q-card-actions>
    </q-card>
    <!-- END OF LOGIN -->

    <!-- SIGN UP -->
    <q-card style="min-width: 350px" v-if="signUp">
      <q-card-section class="row items-center q-gutter-md bg-primary">
        <CockatriceAvatar square size="25px" />
        <div class="text-h6" style="color: white">Sign Up</div>
      </q-card-section>

      <q-card-section class="q-pt-none column q-pa-lg q-gutter-lg">
        <q-input label-color="primary" v-model="email" label="Email" autofocus />
        <q-input label-color="primary" v-model="username" label="Username" />
        <q-input v-model="password" label="Password" type="password" label-color="primary" />
      </q-card-section>

      <q-card-actions align="right" class="text-primary">
        <q-btn flat label="Cancel" @click="signUp = false" />
        <q-btn flat label="Submit" />
      </q-card-actions>
    </q-card>
    <!-- END OF SIGN UP -->
  </q-dialog>
</template>

<script setup lang="ts">
import { ref } from 'vue';
import CockatriceAvatar from 'components/CockatriceAvatar.vue';
import { useRouter } from 'vue-router';

const router = useRouter();

const loginDialog = ref(true);
const signUp = ref(false);
const username = ref('');
const email = ref('');
const password = ref('');
const savePassword = ref(false);
const serverOptions = ref([
  { label: 'Rooster Ranges', port: '1234' },
  { label: 'Chickatrice', port: '1234' },
]);
const serverDropdown = ref('');
async function login() {
  await router.push({ name: 'home' });
  loginDialog.value = false;
}
</script>
