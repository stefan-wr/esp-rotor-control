<template>
  <Transition
    name="slide"
    @before-enter="$emit('before-enter')"
    @enter="$emit('enter')"
    @after-enter="$emit('after-enter')"
    @before-leave="$emit('before-leave')"
    @leave="$emit('leave')"
    @after-leave="$emit('after-leave')"
    v-show="toggle"
  >
    <slot></slot>
  </Transition>
</template>

<script setup>
// Prop toggle: false -> hidden
const props = defineProps({
  toggle: {
    type: Boolean,
    default: false,
    required: true
  }
});

// Passthrough events of transition-component to parent components
const emit = defineEmits([
  'before-enter',
  'enter',
  'after-enter',
  'before-leave',
  'leave',
  'after-leave'
]);
</script>

<style lang="scss" scoped>
/* Slide-in transition */
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
