import { fileURLToPath, URL } from 'node:url'
import { resolve, dirname } from 'node:path'
import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import { viteSingleFile } from 'vite-plugin-singlefile'
import VueI18nPlugin from '@intlify/unplugin-vue-i18n/vite'
import { compression } from 'vite-plugin-compression2';
import rotorControl from './vite-plugins/vite-plugin-rotor-control'

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [
    vue(),
    viteSingleFile(),
    VueI18nPlugin({
      include: resolve(dirname(fileURLToPath(import.meta.url)), './src/locales/**'),
    }),
    compression({
      include: /^(index\.html)$/,
      deleteOriginalAssets: true,
      filename: '[base].gzip'
    }),
    rotorControl({
      source: 'dist/index.html.gzip',
      filename: 'AppIndex.h',
      destination: '../lib/App'
    })
  ],
  
  server: {
    host: true,
    watch: {
      usePolling: true
    }
  },

  build: {
    outDir: "dist/",
    assetsDir: "",
    rollupOptions: {
      output: {
        entryFileNames: `[name].js`,
        chunkFileNames: `[name].js`,
        assetFileNames: `assets/[name].[ext]`
      }
    }
  },

  resolve: {
    alias: {
      '@': fileURLToPath(new URL('./src', import.meta.url)),
      '@root': fileURLToPath(new URL('./', import.meta.url))
    }
  },

  css: {
    preprocessorOptions: {
      scss: {
        api: 'modern-compiler',
        additionalData: '@use "@/assets/_variables.scss" as *;'
      }
    }
  }
})
