<template>
  <div class="card border-box l-align" :class="{ 'card-show': show }">
    <div class="card-header flex-csp medium">
      <span class="card-title">{{ title }}</span>
      <button
        @click="show = !show"
        class="card-hide-btn flex-cc"
        :title="`Karte '${title}' ein-/ausklappen`"
      >
        <svg viewBox="0 0 300 300">
          <path :class="{ rotate: !show }" class="caret-path"
            d="M136.874,96.18C144.4,89.634 155.6,89.634 163.126,96.18C186.786,116.76 237.15,160.568 269.744,188.919C274.465,193.026 276.141,199.632 273.949,205.492C271.757,211.353 266.157,215.237 259.899,215.237C202.495,215.237 97.505,215.237 40.101,215.237C33.843,215.237 28.243,211.353 26.051,205.492C23.859,199.632 25.535,193.026 30.256,188.919C62.85,160.568 113.214,116.76 136.874,96.18Z"
            style="fill: var(--text-color)"
          />
        </svg>
      </button>
    </div>
    <div v-if="true" class="card-slider">
      <div class="card-content">
        <slot> </slot>
      </div>
    </div>
  </div>
</template>

<script setup>
import { reactive, ref, computed, onUpdated, onMounted } from 'vue';

const props = defineProps({
  title: String
});

const show = ref(true);

// Set the negative top margin that is used to
// hide the card to the exact height of the card.
function setSlideMargin() {
  // Get correct card first
  const cards = document.getElementsByClassName('card');
  for (let i = 0; i < cards.length; i++) {
    const card = cards.item(i);
    if (card.firstChild.firstChild.textContent === props.title) {
      // Add margin to content of the card
      const content = card.childNodes.item(1).firstChild;
      content.style.marginTop = `${content.offsetHeight * -1 - 1}px`;
    }
  }
}

onUpdated(() => {
  setSlideMargin();
});

onMounted(() => {
  setSlideMargin();
});
</script>

<style lang="scss" scoped>
$card-trans-spd: 0.2s;

.card {
  width: 100%;
  display: flex;
  flex-direction: column;
  justify-content: left;
}

.card-header {
  gap: 1em;
}

.card-hide-btn {
  background: transparent;
  color: var(--text-color);
  padding: 0.3em;
}

.card-hide-btn svg {
  height: 1em;
}

.caret-path {
  transform: rotateZ(0deg);
  transition: transform $card-trans-spd;
  transform-origin: center;
}

.rotate {
  transform: rotateZ(180deg) !important;
}

.card-slider {
  overflow: hidden;
}

.card-content {
  width: 100%;
  margin-top: -100%;
  opacity: 0;
  transform: scaleX(90%);
  transform-origin: bottom center;
  transition: margin-top $card-trans-spd, opacity $card-trans-spd, transform $card-trans-spd;
}

.card-show .card-content {
  margin-top: 1em !important;
  opacity: 1;
  transform: scaleX(100%);
}
</style>
