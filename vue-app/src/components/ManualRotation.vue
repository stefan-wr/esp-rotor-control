<template>
  <Card title="Manuelle Rotation">
    <div class="flex-cc buttons-wrap">
      <button
        class="medium bold"
        :class="{ 'btn-pressed': isLeftBtnPressed }"
        @mousedown.prevent="rotateLeft()"
        @touchstart.prevent="rotateLeft()"
        @mouseup.prevent="stopRotation()"
        @mouseleave.prevent="stopRotation()"
        @touchend.prevent="stopRotation()"
        @touchcancel.prevent="stopRotation()"
      >
      <Icon icon="fa-solid fa-rotate-left" />
      </button>

      <button class="medium bold" @click.prevent="stopRotation()">Stop</button>

      <button
        class="medium bold"
        :class="{ 'btn-pressed': isRightBtnPressed }"
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

import { ref, reactive, computed, onMounted } from 'vue';

import { useUmbrellaStore } from '@/stores/umbrella';
import { useRotorStore } from '@/stores/rotor';
import { useUIStore } from '@/stores/ui';

const umbrellaStore = useUmbrellaStore();
const rotorStore = useRotorStore();
const uiStore = useUIStore();

let isLeftBtnPressed = ref(false);
let isRightBtnPressed = ref(false);

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

onMounted(() => {
  // Add arrow left/right listeners for manual rotation
  if (!uiStore.ui.hasRotationKeyListener) {
    // >>>>> Arrow Keys, key-down
    document.addEventListener('keydown', function (event) {
      if (!event.repeat && uiStore.ui.kbscEnabled) {
        switch (event.key) {
          case 'ArrowLeft':
            rotateLeft();
            break;
          case 'ArrowRight':
            rotateRight();
            break;
        }
      }
    });

    // >>>>> Arrow Keys, key-up;
    document.addEventListener('keyup', function (event) {
      if (uiStore.ui.kbscEnabled) {
        if (['ArrowLeft', 'ArrowRight'].includes(event.key)) {
          stopRotation();
        }
      }
    });
    uiStore.ui.hasRotationKeyListener = true;
  }
});
</script>

<style lang="scss">
.buttons-wrap {
  gap: 1em;
  justify-content: stretch;
}

.buttons-wrap button {
  flex-grow: 1;
  padding-top: 0.5em;
  padding-bottom: 0.5em;
}
</style>
