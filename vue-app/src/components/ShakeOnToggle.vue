<template>
  <div :class="{ shake: isShaking }">
    <slot></slot>
  </div>
</template>

<script setup>
import { ref, watch } from 'vue';
const emit = defineEmits(['before-shake', 'after-shake']);

// Prop 'toggle': false -> true: shake
const props = defineProps({
  toggle: {
    type: Boolean,
    default: false
  }
});

// Shake this component, emit start and stop events
const isShaking = ref(false);

function shake() {
  emit('before-shake');
  isShaking.value = true;
  setTimeout(() => {
    isShaking.value = false;
    emit('after-shake');
  }, 300);
}

// Expose shake function to parent scope
defineExpose({ shake });

// Shake when prop.toggle changes to true
watch(
  () => props.toggle,
  (newValue) => {
    if (newValue) {
      shake();
    }
  }
);
</script>

<style scoped>
.shake {
  animation: shake 0.3s;
}

@keyframes shake {
  0% {
    transform: translateX(0%);
  }
  20% {
    transform: translateX(-8%);
  }
  40% {
    transform: translateX(5%);
  }
  60% {
    transform: translateX(-3%);
  }
  80% {
    transform: translateX(2%);
  }
  100% {
    transform: translateX(0%);
  }
}
</style>
