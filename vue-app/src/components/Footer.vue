<template>
  <footer id="footer" class="border-box flex-cc">
    <span class="small">
      RotorControl UI - v0.0.1 - made by
      <a href="https://github.com/stefan-wr">Stefan Wraase</a>
    </span>
  </footer>
</template>

<style lang="scss" scoped>
#footer {
  padding: 0 1em !important;
  height: calc($border-box-bradius * 2);
  background-color: var(--content-color-0);
  color: var(--content-color-3);

  span {
    filter: brightness(120%);
  }

  a {
    text-decoration: underline;

    &:hover {
      color: var(--text-color);
    }
  }
}
</style>
