<template>
  <div class="trans-wrap" ref="transWrap">
    <Transition name="childA-trans" @enter="applyCurrentWrapHeight" v-show="!toggle">
      <slot name="childA"></slot>
    </Transition>

    <!-- New favorite form -->
    <Transition name="childB-trans" @enter="applyCurrentWrapHeight" v-show="toggle">
      <slot name="childB"></slot>
    </Transition>
  </div>
</template>

<script setup>
import { ref, onMounted, onBeforeUnmount } from 'vue';

// Toggle prop: false -> show Child A, true -> show Child B
const props = defineProps({
  toggle: false
});

// Transition between child-A and child-B.
// For a smooth transition, adjust the height of the wrapping
// div to the height of the new child when it starts to fly in.
// -------------------------------------------------------------
const transWrap = ref(null);

// Initially, set wrapper height to height of initial child and
// reapply height when the browser window resizes
onMounted(() => {
  applyCurrentWrapHeight();
  window.addEventListener('resize', applyCurrentWrapHeight);
});

// Rmove resize event listener when components is unmounted
onBeforeUnmount(() => {
  window.removeEventListener('resize', applyCurrentWrapHeight);
});

// Adjust height of wrapper to height of currently shown child
function applyCurrentWrapHeight() {
  if (props.toggle) {
    const childB = transWrap.value.children.item(1);
    transWrap.value.style.height = String(childB.offsetHeight) + 'px';
  } else {
    const childA = transWrap.value.children.item(0);
    transWrap.value.style.height = String(childA.offsetHeight) + 'px';
  }
}
</script>

<style lang="scss" scoped>
/* Vue Transition animation, sliding in
   and out child-B and child-A */
$speed: 0.2s;
.trans-wrap {
  transition: height $speed;
  position: relative;
  overflow: hidden;
}

/* Child A */
.childA-trans-leave-active,
.childA-trans-enter-active {
  transition: transform $speed, opacity $speed;
  position: absolute;
  top: 0;
  left: 0;
}
.childA-trans-leave-to,
.childA-trans-enter-from {
  opacity: 0;
  transform: translateX(-80%) scaleY(70%);
}

/* Child B */
.childB-trans-leave-active,
.childB-trans-enter-active {
  transition: transform $speed, opacity $speed;
  position: absolute;
  top: 0;
  left: 0;
}
.childB-trans-leave-to,
.childB-trans-enter-from {
  opacity: 0;
  transform: translateX(80%) scaleY(70%);
}
</style>
