<template>
  <div class="large flex-cc" style="padding: 2em">
    {{ message }}
  </div>
</template>

<script setup>
import { onMounted } from 'vue';
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

const thisRouteName = router.currentRoute.value.name;

onMounted(() => {
  if (props.pushTo) {
    setTimeout(function () {
      if (router.currentRoute.value.name === thisRouteName) {
        router.push({ name: props.pushTo });
      }
    }, props.pushAfter);
  }
});
</script>
