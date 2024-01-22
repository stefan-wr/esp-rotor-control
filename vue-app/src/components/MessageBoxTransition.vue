<template>
  <Transition
    name="slide"
    @before-enter="$emit('before-transition')"
    @enter="$emit('while-transition')"
    @after-enter="shakeForm()"
    @before-leave="$emit('before-transition')"
    @leave="$emit('while-transition')"
    @after-leave="$emit('after-transition')"
    v-show="toggle"
  >
    <slot :isShaking="isShaking"> </slot>
  </Transition>
</template>

<script setup>
import { ref } from 'vue';

// Props
const props = defineProps({
  toggle: {
    type: Boolean,
    default: false,
    required: true
  },
  shake: {
    type: Boolean,
    default: false
  }
});

// Shake form
const isShaking = ref(false);

function shakeForm() {
  emit('while-transition');
  if (props.shake) {
    isShaking.value = true;
    setTimeout(() => {
      isShaking.value = false;
    }, 300);
  }
}

// Emits
const emit = defineEmits(['while-transition', 'before-transition', 'after-transition']);
</script>

<style lang="scss" scoped>
.slide-leave-active,
.slide-enter-active {
  transition:
    transform $slide-speed,
    opacity $slide-speed;
}
.slide-leave-to,
.slide-enter-from {
  opacity: 0;
  transform: translateX(-80%) scaleY(70%);
}
</style>
