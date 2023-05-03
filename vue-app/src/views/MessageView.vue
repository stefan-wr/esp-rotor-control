<template>
  <div class="large flex-cc" style="padding: 2em">
    {{ message }}
  </div>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue';
import { useRouter } from 'vue-router';

const router = useRouter();

const props = defineProps({
  message: {
    type: String,
    required: true
  },
  pushAfter: {
    type: Number,
    required: true
  },
  pushTo: {
    type: String,
    required: true
  }
});

onMounted(() => {
  if (props.pushTo) {
    setTimeout(function () {
      router.push({ name: props.pushTo });
    }, props.pushAfter);
  }
});
</script>
