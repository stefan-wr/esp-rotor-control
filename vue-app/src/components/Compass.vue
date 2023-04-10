<template>
  <div class="compass border-box">
    <div id="compass-svg">
      <div v-if="overlap" class="overlap bold medium">OL</div>
      <svg id="compass" viewBox="0 0 1000 1000" style="fill-rule: evenodd">
        <path
          d="M500,0C775.958,0 1000,224.042 1000,500C1000,775.958 775.958,1000 500,1000C224.042,1000 0,775.958 0,500C0,224.042 224.042,0 500,0ZM500,50C748.362,50 950,251.638 950,500C950,748.362 748.362,950 500,950C251.638,950 50,748.362 50,500C50,251.638 251.638,50 500,50Z"
          style="fill: #f7f7f7"
        />
        <g id="cmp-req-needle" :style="{ transform: 'rotate(' + req_angle + 'deg)' }">
          <path d="M500,15L515,40L515,700L485,700L485,40L" style="fill: var(--text-color-accent)" />
        </g>
        <g id="cmp-needle" :style="{ transform: 'rotate(' + store.getAngle1D + 'deg)' }">
          <path d="M500,200L550,500L500,650L450,500L500,200" style="fill: #f7f7f7" />
        </g>
      </svg>
    </div>
    <div class="compass-label border-box">
      <span id="cmp_angle" class="large"></span><span class="large">{{ store.getAngle2D }}°</span
      ><br />
      <span id="cmp_cardinal" class="large">{{ store.getCardinal }}</span
      ><br />
      <span>--</span> <span id="cmp_req_angle" class=""></span><span>°</span>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue';
import { useRotorStore } from '@/stores/rotor';

const store = useRotorStore();

const req_angle = ref(0);

function setRequestAngle(event) {
  var rect = document.getElementById("compass-svg").getBoundingClientRect();
  var x = event.offsetX - rect.width / 2;
  var y = (event.offsetY - rect.height / 2) * -1;
  var angle = (Math.atan2(x, y) * 180) / Math.PI;
  if (angle < 0) {
    angle = 360 + angle;
  }
  req_angle.value = angle;
}

const overlap = computed(() => {
  if (store.rotor.angle > 360.0) {
    return true;
  }
  return false;
});

onMounted(() => {
  document.getElementById("compass-svg").addEventListener("mousemove", (event) =>{
    console.log(event.target.id);
    setRequestAngle(event);
  });
});
</script>

<style scoped>
#compass-svg {
  margin-bottom: 1em;
  position: relative;
  z-index: 1;
  cursor: pointer;
}

.overlap {
  position: absolute;
  top: 0;
  right: 0;
  color: var(--alert-color);
  border: 0.2em solid var(--alert-color);
  border-radius: 0.5em;
  padding: 0.25em;
  z-index: -1;
}

#cmp-req-needle {
  transform-origin: center;
  transform: rotate(0deg);
}

#cmp-needle {
  transform-origin: center;
  transform: rotate(0deg);
  transition: transform 0.1s;
}
</style>
