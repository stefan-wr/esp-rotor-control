<template>
  <div class="flex-vst gap-one border-box">
    <div class="flex-hc gap-one">
      <div class="flex-cl gap-half flex-grow wrd-break">
        <slot name="icon"></slot>
        {{ title }}
      </div>
      <slot name="action"></slot>
    </div>
    <slot name="content"></slot>
  </div>
</template>

<script setup>
const props = defineProps({
  title: {
    type: String,
    required: true
  }
});
</script>
