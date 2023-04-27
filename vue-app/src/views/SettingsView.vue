<template>
    <main class="gap-one">
      <Settings></Settings>
      <Calibration></Calibration>
      <Footer id="footer"></Footer>
    </main>
</template>

<script setup>
import Settings from '@/components/settings/Settings.vue';
import Calibration from '@/components/settings/Calibration.vue';
import Footer from '@/components/Footer.vue';
</script>

<style lang="scss" scoped>
main {
  display: grid;
  align-items: start;
  justify-items: stretch;
  grid-template-columns: minmax(0, 1fr) minmax(0, 1fr);

  @include medium {
    display: flex;
    flex-direction: column;
    align-items: start;
  }
}

#footer {
  grid-column: 1 / 3;
  width: 100%;
}
</style>
