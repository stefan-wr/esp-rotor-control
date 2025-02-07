<template>
    <div class="flex-vst gap-one ">
        <h2 class="medium hide-m flex-self-start">{{ $t('settings.licenses') }}</h2>
        <div class="md-wrap border-box">
          <vue-markdown class="md-container l-align" :source="rotorcontrol" />
        </div>
        <div class="md-wrap border-box">
          <vue-markdown class="md-container l-align" :source="fontawesome" />
        </div>
        <div class="md-wrap border-box">
          <vue-markdown class="md-container l-align" :source="inter" />
        </div>
    </div>
</template>

<style lang="scss">
.md-container {
width: min(60ch, 100%);

  p {
    margin-top: 1em;
  }

  ul, ol {
    margin-top: 1em;
    margin-bottom: 1em;
    padding-left: 2em;
  }

  ul {
    list-style-type: disc;
  }

  hr {
    margin-top: 2em;
    margin-bottom: 2em;
  }

  a {
    text-decoration: underline;

    &:hover, &:focus-visible {
        color: var(--text-color-darker);
    }
  }
}
</style>

<script setup>
import VueMarkdown from 'vue-markdown-render';

const rotorcontrol =
`
### RotorControl
RotorControl is licensed under the [Apache 2.0 License](https://github.com/stefan-wr/esp-rotor-control/blob/main/LICENSE).

Github: [https://github.com/stefan-wr/esp-rotor-control](https://github.com/stefan-wr/esp-rotor-control)
`

const fontawesome =
`
### Fontawesome
This project uses free Fontawesome icons [https://fontawesome.com](https://fontawesome.com).

Fontawesome icons are licensed under [CC BY 4.0 License](https://creativecommons.org/licenses/by/4.0/).

Fontawesome code is licensed under [MIT License](https://opensource.org/license/MIT).
`

const inter = 
`
### Inter Font
This project uses the Inter fonts ([https://github.com/rsms/inter](https://github.com/rsms/inter)), released
under the following license:

Copyright (c) 2016 The Inter Project Authors ([https://github.com/rsms/inter](https://github.com/rsms/inter))

This Font Software is licensed under the SIL Open Font License, Version 1.1.
This license is available with a FAQ at:

[http://scripts.sil.org/OFL](http://scripts.sil.org/OFL)
`
</script>
