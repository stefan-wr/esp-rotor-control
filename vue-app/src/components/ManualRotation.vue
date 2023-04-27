<template>
  <Card title="Manuelle Rotation">
    <template #icon><Icon icon="fa-solid fa-arrows-spin"></Icon></template>
    <div class="flex-hc gap-one buttons-wrap">
      <button
        class="medium bold"
        :class="{ 'btn-pressed': isLeftBtnPressed }"
        title="Drehe den Rotor gegen den Uhrzeigersinn"
        @mousedown.prevent="rotateLeft()"
        @touchstart.prevent="rotateLeft()"
        @mouseup.prevent="stopRotation()"
        @mouseleave.prevent="stopRotation()"
        @touchend.prevent="stopRotation()"
        @touchcancel.prevent="stopRotation()"
      >
        <Icon icon="fa-solid fa-rotate-left" />
      </button>

      <button class="medium bold" title="Stoppe den Rotor" @click.prevent="stopRotation()">
        Stop
      </button>

      <button
        class="medium bold"
        :class="{ 'btn-pressed': isRightBtnPressed }"
        title="Drehe den Rotor mit dem Uhrzeigersinn"
        @mousedown.prevent="rotateRight()"
        @touchstart.prevent="rotateRight()"
        @mouseup.prevent="stopRotation()"
        @mouseleave.prevent="stopRotation()"
        @touchend.prevent="stopRotation()"
        @touchcancel.prevent="stopRotation()"
      >
        <Icon icon="fa-solid fa-rotate-right" />
      </button>
    </div>
  </Card>
</template>

<script setup>
import Card from '@/components/Card.vue';

import { ref, reactive, computed, onMounted, onBeforeUnmount } from 'vue';

import { useUmbrellaStore } from '@/stores/umbrella';
import { useRotorStore } from '@/stores/rotor';
import { useUIStore } from '@/stores/ui';

const umbrellaStore = useUmbrellaStore();
const rotorStore = useRotorStore();
const uiStore = useUIStore();

const isLeftBtnPressed = ref(false);
const isRightBtnPressed = ref(false);

function rotateLeft() {
  stopRotation();
  isLeftBtnPressed.value = true;
  umbrellaStore.sendRotation(-1);
}

function rotateRight() {
  stopRotation();
  isRightBtnPressed.value = true;
  umbrellaStore.sendRotation(1);
}

function stopRotation() {
  if (isLeftBtnPressed.value || isRightBtnPressed.value) {
    isLeftBtnPressed.value = false;
    isRightBtnPressed.value = false;
    umbrellaStore.sendRotation(0);
  }
}

function startRotationKeyEventListener(event) {
  if (!event.repeat && uiStore.ui.kbscEnabled && event.target.tagName !== 'INPUT') {
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

function stopRotationKeyEventListener(event) {
  if (uiStore.ui.kbscEnabled && event.target.tagName !== 'INPUT') {
    if (['ArrowLeft', 'ArrowRight'].includes(event.key)) {
      stopRotation();
    }
  }
}

onMounted(() => {
  // Add arrow left/right listeners for manual rotation
  document.addEventListener('keydown', startRotationKeyEventListener);
  document.addEventListener('keyup', stopRotationKeyEventListener);
});

onBeforeUnmount(() => {
  // Remove arrow left/right listeners
  document.removeEventListener('keydown', startRotationKeyEventListener);
  document.removeEventListener('keyup', stopRotationKeyEventListener);
});
</script>

<style lang="scss">
.buttons-wrap button {
  flex: 1 1 33%;
  padding-top: 0.5em;
  padding-bottom: 0.5em;
}
</style>
