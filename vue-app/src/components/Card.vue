<template>
  <div class="card border-box l-align" :class="{ 'card-show': uiStore.ui.cards[props.title] }">
    <div class="flex-csp gap-half medium">
      <slot name="icon"></slot>
      <span class="card-title bold">{{ title }}</span>
      <button
        class="card-hide-btn flex-cc"
        @click="toggleCollapse"
        :title="`Bereich '${title}' ein-/ausklappen`"
        :aria-label="`Bereich '${title}' ein-/ausklappen`"
      >
        <svg viewBox="0 0 300 300">
          <path
            :class="{ 'caret-rot': !uiStore.ui.cards[props.title] }"
            class="caret-path"
            d="M136.874,96.18C144.4,89.634 155.6,89.634 163.126,96.18C186.786,116.76 237.15,160.568 269.744,188.919C274.465,193.026 276.141,199.632 273.949,205.492C271.757,211.353 266.157,215.237 259.899,215.237C202.495,215.237 97.505,215.237 40.101,215.237C33.843,215.237 28.243,211.353 26.051,205.492C23.859,199.632 25.535,193.026 30.256,188.919C62.85,160.568 113.214,116.76 136.874,96.18Z"
            style="fill: var(--text-color)"
          />
        </svg>
      </button>
    </div>
    <div v-if="true" class="card-slider">
      <div
        class="card-content"
        ref="cardContent"
        :class="{ 'card-content-trans': hasTransitionClass }"
      >
        <slot></slot>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onUpdated, onMounted } from 'vue';
import { useUIStore } from '../stores/ui';

const uiStore = useUIStore();

const props = defineProps({
  title: String
});

// Element ref
const cardContent = ref(null);

/**
 * Used to only add CSS transitions to card content
 * when it is mounted and not collapsed:
 *  1. On mount: add transition 200ms after mount if not collapsed
 *  2. Collapse: remove transition 200ms after collapse
 *  3. Show: add transition class immediately
 */
const hasTransitionClass = ref(false);

/** Toggle collapse card content. After animation played:
 *  - set fixed height to hidden card content
 *  - remove CSS transitions from card content
 * This avoids ressizing of hidden content to affect the collapsed card
 */
function toggleCollapse() {
  if (uiStore.ui.cards[props.title]) {
    // Hide
    uiStore.ui.cards[props.title] = false;
    setTimeout(() => {
      hasTransitionClass.value = false;
      cardContent.value.style.height = `${cardContent.value.offsetHeight}px`;
    }, 200);
  } else {
    // Show
    uiStore.ui.cards[props.title] = true;
    cardContent.value.style.height = null;
    hasTransitionClass.value = true;
  }
}

/** Set the negative top margin that is used to hide the
 *  card content to the exact height of the content -1px.
 */
function setSlideMargin() {
  if (!uiStore.ui.cards[props.title]) {
    cardContent.value.style.marginTop = `${cardContent.value.offsetHeight * -1}px`;
  }
}

onMounted(() => {
  setSlideMargin();
  // Add collapse-content toggle to uiStore on first mount
  if (!Object.hasOwn(uiStore.ui.cards, props.title)) {
    uiStore.ui.cards[props.title] = true;
  }

  // Add transition class 200ms after mount if content is not collapsed
  if (uiStore.ui.cards[props.title]) {
    setTimeout(() => {
      hasTransitionClass.value = true;
    }, 200);
  }
});

// Reapply negative top margin on updates
onUpdated(() => {
  setSlideMargin();
});
</script>

<style lang="scss" scoped>

.card {
  width: 100%;
}

.card-title {
  flex-grow: 1;
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

.caret-rot {
  transform: rotateZ(180deg);
}

.card-slider {
  overflow: hidden;
}

.card-content {
  width: 100%;
  opacity: 0;
  transform: scaleX(90%);
  transform-origin: bottom center;
}

.card-content-trans {
  transition: margin-top $card-trans-spd, opacity $card-trans-spd, transform $card-trans-spd;
}

.card-show .card-content {
  height: 100%;
  margin-top: 1em !important;
  margin-bottom: 1px;
  opacity: 1;
  transform: scaleX(100%);
}
</style>
