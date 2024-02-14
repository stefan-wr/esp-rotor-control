<template>
  <SettingCard :title="$t('settings.language.title')">
    <template #icon>
      <Icon icon="fa-solid fa-font"></Icon>
    </template>

    <template #content>
      <p class="txt-dark">
        {{ $t('settings.language.dscr') }}
      </p>
      <div class="flex-cst gap-half color-themes">
        <button
          v-for="locale in $i18n.availableLocales"
          class="flex-cst gap-half btn-std-resp theme"
          :class="{ 'theme-active': uiStore.isActiveLocale(locale) }"
          @click="uiStore.setLocale(locale)"
          :disabled="uiStore.isActiveLocale(locale)"
        >
          <span class="no-wrap-ellip">{{ $t('locales.' + locale) }}</span>
          <Transition name="icon-trans">
            <Icon
              v-if="uiStore.isActiveLocale(locale)"
              icon="fa-solid fa-check"
              class="active-icon"
            ></Icon>
          </Transition>
        </button>
      </div>
    </template>
  </SettingCard>
</template>

<script setup>
import SettingCard from '@/components/settings/SettingCard.vue';
import { useUIStore } from '@/stores/ui';
const uiStore = useUIStore();
</script>

<style lang="scss" scoped>
.color-themes {
  flex-wrap: wrap;
}

.theme {
  min-width: 0px;
  flex-grow: 1;
  flex-shrink: 1;
  flex-basis: 48%;
  text-align: left;
  padding-top: 0.7em;
  padding-bottom: 0.7em;
  background-color: var(--content-color-1);
  color: var(--text-color);
  transition: background-color 0.1s;

  &:hover {
    background-color: var(--content-color-3);
    filter: unset;
  }

  &:active {
    transform: unset;
  }

  &:disabled {
    filter: unset;
  }

  span {
    color: var(--text-color-darker);
    flex-grow: 1;
  }

  &:hover {
    span {
      color: var(--text-color);
    }
  }
}

.theme-active {
  background-color: var(--content-color-3);
  cursor: default;

  &:hover {
    filter: none;
  }

  span {
    color: var(--text-color);
    opacity: 1;
  }
}

.active-icon {
  transform: center;
}

.icon-trans-enter-active {
  transition:
    transform 0.2s,
    opacity 0.2s;
  transform: translateX(0);
}
.icon-trans-enter-from {
  transform: translateX(1em);
  opacity: 0;
}

.icon-trans-leave-active {
  transition:
    transform 0.1s,
    opacity 0.1s;
}

.icon-trans-leave-to {
  transform: scaleY(100%) translateX(1em);
  opacity: 0;
}
</style>
