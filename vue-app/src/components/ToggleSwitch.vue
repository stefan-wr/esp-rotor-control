<template>
  <div class="switch" :class="{ disabled: disabled }">
    <input
      type="checkbox"
      @change="$emit('toggle')"
      autocomplete="off"
      :class="{ disabled: disabled }"
      :checked="toggle"
      :disabled="disabled"
    />
    <span
      class="switch-slider"
      tabindex="0"
      @keyup.enter="$emit('toggle')"
      @keyup.space="$emit('toggle')"
    ></span>
    <span class="switch-on small">
      <slot name="on">AN</slot>
    </span>
    <span class="switch-off small">
      <slot name="off">AUS</slot>
    </span>
  </div>
</template>

<script setup>
const props = defineProps({
  toggle: {
    type: Boolean,
    required: true
  },
  disabled: {
    type: Boolean,
    default: false
  }
});
</script>

<style lang="scss" scoped>
/* Toggle switch */
$slider-trans: 0.2s;

.switch {
  /*
  --switch-height: 32px;
  --switch-width: calc(var(--switch-height) * 2);
  --switch-padding: 4px;
  --switch-bradius: 6px;
  --switch-thumb-size: calc(var(--switch-height) - 2 * var(--switch-padding));
  --switch-thumb-bradius: 4px;
  --switch-thumb-translateX: calc(
    var(--switch-width) - 2 * var(--switch-padding) - var(--switch-thumb-size)
  );
  */

  --switch-height: 2.3em;
  --switch-width: calc(var(--switch-height) * 2);
  --switch-padding: 0.3em;
  --switch-bradius: 0.4em;
  --switch-thumb-size: calc(var(--switch-height) - 2 * var(--switch-padding));
  --switch-thumb-bradius: 0.3em;
  --switch-thumb-translateX: calc(
    var(--switch-width) - 2 * var(--switch-padding) - var(--switch-thumb-size)
  );

  position: relative;
  width: var(--switch-width);
  height: var(--switch-height);
  flex-shrink: 0;

  @media (hover: none) {
    --switch-height: 2em;
    font-size: 1.1rem !important;

    @include medium {
      font-size: 1.2rem !important;
    }

    @include small {
      font-size: 1.2rem !important;
    }
  }
}

.switch input {
  cursor: pointer;
  position: relative;
  z-index: 1;
  width: 100%;
  height: 100%;
  padding: 0;
  margin: 0;
  border: none;
  opacity: 0;
}

.disabled {
  filter: saturate(10%);
  cursor: not-allowed !important;
}

/* Full switch border box*/
.switch > .switch-slider {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  border-radius: var(--switch-bradius);
  background-color: var(--content-color-1);
  transition: background-color $slider-trans;

  &:focus-visible {
    border-radius: var(--switch-bradius);
    outline: 0.2em solid var(--accent-color);
    outline-offset: 0.2em;
  }
}

.switch input:checked + .switch-slider {
  background-color: var(--accent-color);
}

/* Switch thumb */
.switch > .switch-slider:before {
  position: absolute;
  content: '';
  height: var(--switch-thumb-size);
  width: var(--switch-thumb-size);
  left: var(--switch-padding);
  top: var(--switch-padding);
  background-color: var(--text-color);
  transition: transform $slider-trans;
  border-radius: var(--switch-thumb-bradius);
}

.switch > input:hover ~ .switch-slider:before {
  filter: brightness(0.9);
}

.switch input:checked + .switch-slider:before {
  background-color: var(--text-color-accent);
  transform: translateX(var(--switch-thumb-translateX));
}

/* On/Off Labels*/
.switch > .switch-on,
.switch > .switch-off {
  position: absolute;
  top: 50%;
  transform: translateY(-50%);
  transition: opacity $slider-trans;
}

.switch > .switch-on {
  left: calc(var(--switch-padding) + 0.3em);
  color: var(--text-color-accent);
  margin-left: 0.2em;
  opacity: 0;
}

.switch > .switch-off {
  right: calc(var(--switch-padding) + 0.3em);
  margin-right: 0.2em;
  opacity: 0.2;
}

.switch input:checked ~ .switch-on {
  opacity: 0.6;
}

.switch input:checked ~ .switch-off {
  opacity: 0;
}
</style>
