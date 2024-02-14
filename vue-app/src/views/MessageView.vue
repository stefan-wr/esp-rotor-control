<template>
  <!-- Message with pushAfter -->
  <div class="flex-vc gap-one message" v-if="props.pushTo">
    <p class="large">{{ $t(message) }}</p>
    <p class="txt-dark">
      {{ $t('messages.autoReload') }}<br />
      <span class="large txt-dark"> {{ $t('messages.seconds', { n: seconds }) }}</span>
    </p>
  </div>

  <!-- Message without pushAfter -->
  <div class="large flex-vc message" v-else>
    <p>{{ $t(message) }}</p>
  </div>
</template>

<script setup>
import { toRef, onMounted } from 'vue';
import { useRouter } from 'vue-router';
import { useUIStore } from '@/stores/ui';

const router = useRouter();
const uiStore = useUIStore();

const props = defineProps({
  message: {
    type: String,
    required: true
  },
  pushTo: {
    type: String,
    default: ''
  },
  pushAfter: {
    type: Number,
    default: 0
  },
  disableHeaderLinks: {
    type: Boolean,
    default: false
  },
  reload: {
    type: Boolean,
    default: false
  }
});

const seconds = toRef(props.pushAfter / 1000);
const thisRouteName = router.currentRoute.value.name;

// Decrease seconds and clear interval when 1
let intervalID;

function decreaseSeconds() {
  seconds.value--;
  if (seconds.value === 1) {
    window.clearInterval(intervalID);
  }
}

onMounted(() => {
  // Disable router links in header if wanted
  if (props.disableHeaderLinks) {
    uiStore.disableHeaderLinks();
  }

  // Push to router after x seconds if wanted
  if (props.pushTo) {
    intervalID = setInterval(() => {
      decreaseSeconds();
    }, 1000);
    setTimeout(() => {
      // Push to route
      if (router.currentRoute.value.name === thisRouteName) {
        router.push({ name: props.pushTo });
      }
      // Page reload
      if (props.reload) {
        location.reload();
      }
    }, props.pushAfter);
  }
});
</script>

<style lang="scss" scoped>
.message {
  padding: 2em;

  @include medium {
    padding: 1em;
  }

  @include small {
    padding: 1em;
  }

  // Direct children
  & > * {
    max-width: $break-m;
  }
}
</style>
