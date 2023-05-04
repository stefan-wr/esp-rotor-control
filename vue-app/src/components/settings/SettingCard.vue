<template>
  <div class="flex-vst gap-one border-box">
    <div class="flex-hc gap-one">
      <div class="flex-cl gap-half flex-grow wrd-break">
        <div class="setting-icon"><slot name="icon"></slot></div>
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

<style lang="scss" scoped>
  .setting-icon {
    min-width: 1.3em;
  }
</style>
