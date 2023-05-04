<template>
  <Card id="lock" title="Rotorsperre">
    <template #icon>
      <Icon v-if="settingsStore.lock.isLocked" icon="fa-solid fa-lock"></Icon>
      <Icon v-else icon="fa-solid fa-lock-open"></Icon>
    </template>
    <div class="flex-vst gap-one">
      <p>
        Sperre den Rotor für andere Benutzer. Gebe dazu im Feld unten deinen Name ein und betätige
        den Schalter rechts daneben.
      </p>
      <div class="flex-cst gap-one">
        <input
          class="input-std-resp flex-grow"
          type="text"
          placeholder="Dein Name"
          v-model="settingsStore.lock.name"
          :disabled="settingsStore.lock.isLocked"
          @keypress="restrictInput($event)"
          @paste="restrictPaste($event)"
        />

        <Icon
          icon="fa-solid fa-arrow-right"
          class="medium"
          style="color: var(--content-color-1)"
        ></Icon>

        <ToggleSwitch
          :toggle="settingsStore.lock.isLocked"
          :disabled="settingsStore.isLockedByElse"
          @toggle="toggleLock"
        >
          <template #on><Icon icon="fa-solid fa-lock switch-lbl"></Icon></template>
          <template #off
            ><Icon icon="fa-solid fa-lock-open switch-lbl"></Icon
          ></template>
        </ToggleSwitch>
      </div>
    </div>

    <div class="normal">
      <CardToggleContentTransition :toggle="!settingsStore.lock.isLocked">
        <template #childA>
          <div class="lock-lbl-wrap">
            <div class="border-box flex-cl gap-half">
              <Icon icon="fa-solid fa-lock"></Icon>
              <span>
                Rotor <b>gesperrt</b> von <b>{{ settingsStore.lock.by }}</b>
              </span>
            </div>
          </div>
        </template>

        <template #childB>
          <div style="margin-top: -1em; height: 1px"></div>
        </template>
      </CardToggleContentTransition>
    </div>
  </Card>
</template>

<script setup>
import Card from '@/components/Card.vue';
import ToggleSwitch from '@/components/ToggleSwitch.vue';
import CardToggleContentTransition from '@/components/CardToggleContentTransition.vue';

import { ref, computed, onMounted } from 'vue';

import { useSettingsStore } from '@/stores/settings';
import { useUmbrellaStore } from '@/stores/umbrella';

const settingsStore = useSettingsStore();
const umbrellaStore = useUmbrellaStore();

const toggle = ref(false);
const maxNameLength = 15;

// Toggle lock status
function toggleLock() {
  if (settingsStore.lock.isLocked) {
    // Unlock
    if (settingsStore.lock.name === settingsStore.lock.by) {
      settingsStore.lock.isLocked = false;
      settingsStore.lock.by = '';
      umbrellaStore.sendLock();
      toggle.value = false;
    }
  } else {
    // Lock
    settingsStore.lock.isLocked = true;
    settingsStore.lock.by = settingsStore.lock.name;
    umbrellaStore.sendLock();
  }
}

// Restrict typing-in more than 15 characters
function restrictInput(event) {
  if (settingsStore.lock.name.length >= maxNameLength) {
    event.preventDefault();
  }
}

// Restrict name length to 15 characters AFTER paste
function restrictPaste(event) {
  setTimeout(() => {
    if (settingsStore.lock.name.length > maxNameLength) {
      settingsStore.lock.name = settingsStore.lock.name.substring(0, maxNameLength);
    }
  }, 0);
}
</script>

<style lang="scss" scoped>

.switch-lbl {
  margin-left: 0.6em;
}

.lock-lbl-wrap {
  padding-top: 1em;
  width: 100%;
}
</style>
