<template>
  <div class="card border-box l-align">
    <div class="flex-cl gap-half medium">
      <slot name="icon"></slot>
      <span class="bold">{{ title }}</span>
    </div>
    <div class="card-content" ref="cardContent">
      <slot></slot>
    </div>
  </div>
</template>

<script setup>

defineProps({
  title: String
});
</script>

<style lang="scss" scoped>

.card {
  width: 100%;
}

.card-content {
  height: 100%;
  margin-top: 1em !important;
}
</style>
