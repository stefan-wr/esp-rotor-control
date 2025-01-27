<template>
  <div class="demo flex-vc gap-half">
    <div class="flex-hc gap-half">
      <h1 class="large">Demo-Modus</h1>
      <Icon
        v-if="!showDemo"
        icon="fa-circle-question fa-check"
        class="medium icon"
        @click="showDemo = true"
      />
    </div>
    <div v-if="showDemo" class="demo-info flex-vc gap">
      <p class="normal">
        Die Reaktion des echten Antennen-Rotors wird durch einen Microcontroller emuliert. Dem
        emulierten Rotor stehen nur 4 Geschwindigkeitsstufen und für die Winkelausgabe nur 8 Bit zur
        Verfügung. Firmware-Updates und das Trennen der Netzwerkverbindung sind deaktiviert.
      </p>
      <button class="btn-std-resp bold" @click="showDemo = false">Verstanden</button>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue';
const showDemo = ref(true);
</script>

<style lang="css" scoped>
.demo {
  padding: 1em;
  color: var(--demo-text-color);
}

.icon {
  cursor: pointer;

  &:hover {
    color: var(--demo-text-color-hover);
  }
}

.demo-info {
  max-width: 660px;
}
</style>
