<template>
  <Card title="Manuelle Rotation">
    <template #icon><Icon icon="fa-solid fa-arrows-spin"></Icon></template>
    <div class="flex-hc gap-one buttons-wrap">
      <button
        class="medium bold"
        :class="{ 'btn-pressed': isLeftBtnPressed }"
        title="Dreht den Rotor gegen den Uhrzeigersinn."
        @mousedown.prevent="rotateLeft()"
        @touchstart.prevent="rotateLeft()"
        @mouseup.prevent="stopRotation()"
        @mouseleave.prevent="stopRotationMouseLeave()"
        @touchend.prevent="stopRotation()"
        @touchcancel.prevent="stopRotation()"
        @keydown.space.prevent="startRotationKeyDown(0)"
        @keyup.space.prevent="stopRotationKeyDown()"
        :disabled="settingsStore.isLockedByElse"
      >
        <Icon icon="fa-solid fa-rotate-left" />
      </button>

      <button
        class="medium bold"
        title="Stoppt den Rotor."
        @click.prevent="stopRotation()"
        @keyup.space.prevent="stopRotation()"
        :disabled="settingsStore.isLockedByElse"
      >
        Stop
      </button>

      <button
        class="medium bold"
        :class="{ 'btn-pressed': isRightBtnPressed }"
        title="Dreht den Rotor mit dem Uhrzeigersinn."
        @mousedown.prevent="rotateRight()"
        @touchstart.prevent="rotateRight()"
        @mouseup.prevent="stopRotation()"
        @mouseleave.prevent="stopRotationMouseLeave()"
        @touchend.prevent="stopRotation()"
        @touchcancel.prevent="stopRotation()"
        @keydown.space.prevent="startRotationKeyDown(1)"
        @keyup.space.prevent="stopRotationKeyDown()"
        :disabled="settingsStore.isLockedByElse"
      >
        <Icon icon="fa-solid fa-rotate-right" />
      </button>
    </div>
  </Card>
</template>

<script setup>
import Card from '@/components/Card.vue';

import { callWithAsyncErrorHandling, ref } from 'vue';
import { useEventListener } from '@vueuse/core';

import { useUmbrellaStore } from '@/stores/umbrella';
import { useSettingsStore } from '@/stores/settings';
import { useUIStore } from '@/stores/ui';

const umbrellaStore = useUmbrellaStore();
const settingsStore = useSettingsStore();
const uiStore = useUIStore();

const isLeftBtnPressed = ref(false);
const isRightBtnPressed = ref(false);

function rotateLeft() {
  if (!settingsStore.isLockedByElse) {
    umbrellaStore.sendRotation(0);
    isLeftBtnPressed.value = true;
    umbrellaStore.sendRotation(-1);
  }
}

function rotateRight() {
  if (!settingsStore.isLockedByElse) {
    umbrellaStore.sendRotation(0);
    isRightBtnPressed.value = true;
    umbrellaStore.sendRotation(1);
  }
}

function stopRotation() {
  if (!settingsStore.isLockedByElse) {
    isLeftBtnPressed.value = false;
    isRightBtnPressed.value = false;
    umbrellaStore.sendRotation(0);
  }
}

function stopRotationMouseLeave() {
  if (isLeftBtnPressed.value || isRightBtnPressed.value) {
    stopRotation();
  }
}

// Start/stop rotation by holding/releasing space on tab-selected buttons
let rotationKeyDown = false;
function startRotationKeyDown(dir) {
  if (!rotationKeyDown) {
    rotationKeyDown = true;
    [rotateLeft, rotateRight][dir]();
  }
}

function stopRotationKeyDown() {
  rotationKeyDown = false;
  stopRotation();
}

// Press left/right arrow keys -> start rotation event listener
function startRotationKeyEventListener(event) {
  if (
    !event.repeat &&
    uiStore.ui.kbscEnabled &&
    !settingsStore.isLockedByElse &&
    event.target.tagName !== 'INPUT'
  ) {
    switch (event.key) {
      case 'ArrowLeft':
        rotateLeft();
        break;
      case 'ArrowRight':
        rotateRight();
        break;
    }
  }
}

// Lift left/right arrow keys: stop rotation event listener
function stopRotationKeyEventListener(event) {
  if (uiStore.ui.kbscEnabled && !settingsStore.isLockedByElse && event.target.tagName !== 'INPUT') {
    if (['ArrowLeft', 'ArrowRight'].includes(event.key)) {
      stopRotation();
    }
  }
}

// Register arrow keys event listeners
useEventListener(document, 'keydown', startRotationKeyEventListener);
useEventListener(document, 'keyup', stopRotationKeyEventListener);
</script>

<style lang="scss">
.buttons-wrap button {
  flex: 1 1 33%;
  padding-top: 0.5em;
  padding-bottom: 0.5em;
}
</style>
