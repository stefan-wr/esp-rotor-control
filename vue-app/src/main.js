import { createApp } from 'vue';
import { createPinia } from 'pinia';
import { createI18n } from 'vue-i18n';
import { useUmbrellaStore } from '@/stores/umbrella';
import './assets/main.scss';

import App from './App.vue';
import router from './router';

// FontAwesome icons
import { FontAwesomeIcon } from '@fortawesome/vue-fontawesome';
import { library } from '@fortawesome/fontawesome-svg-core';
import {
    faPlay,
    faPause,
    faStop,
    faRotateRight,
    faRotateLeft,
    faRotate,
    faArrowDown,
    faArrowRight,
    faArrowLeft,
    faArrowsSpin,
    faArrowsRotate,
    faXmark,
    faPlus,
    faMinus,
    faCheck,
    faExclamation,
    faGaugeHigh,
    faCompass,
    faBookmark,
    faGear,
    faWrench,
    faKeyboard,
    faWifi,
    faPowerOff,
    faBolt,
    faTextHeight,
    faLock,
    faLockOpen,
    faPalette,
    faCircleHalfStroke,
    faFile,
    faFileArrowUp,
    faMicrochip,
    faFileImport,
    faCircleInfo,
    faCircleCheck,
    faCircleExclamation,
    faSpinner,
    faExternalLink,
    faDownload,
    faDisplay,
    faFont,
    faCircleQuestion
} from '@fortawesome/free-solid-svg-icons';

library.add(
    faPlay,
    faPause,
    faStop,
    faRotateRight,
    faRotateLeft,
    faRotate,
    faArrowDown,
    faArrowRight,
    faArrowLeft,
    faArrowsSpin,
    faArrowsRotate,
    faXmark,
    faPlus,
    faMinus,
    faCheck,
    faExclamation,
    faGaugeHigh,
    faCompass,
    faBookmark,
    faGear,
    faWrench,
    faKeyboard,
    faWifi,
    faPowerOff,
    faBolt,
    faTextHeight,
    faLock,
    faLockOpen,
    faPalette,
    faCircleHalfStroke,
    faFile,
    faFileArrowUp,
    faMicrochip,
    faFileImport,
    faCircleInfo,
    faCircleCheck,
    faCircleExclamation,
    faSpinner,
    faExternalLink,
    faDownload,
    faDisplay,
    faFont,
    faCircleQuestion
);


// Create app
const app = createApp(App);

// Add fontawesome
app.component('Icon', FontAwesomeIcon);

// Create Pinia
const pinia = createPinia();

// Create i18n with an initial locale.
// Try to get initial locale from browser or localStorage.
// *******************************************************

// Import all i18n resources specified in the plugin `include` option in vite.config.js
import messages from '@intlify/unplugin-vue-i18n/messages'

// Get previously set language from localStorage
let initialLocale = localStorage.getItem('locale');

// Test if locale from localStorage is supported
if (initialLocale == null || !messages.hasOwnProperty(initialLocale)) {
    // It is not supported or null -> first set default locale to 'en'
    initialLocale = 'en';

    // Next, try if browser's prefferd locales are supported.
    try {
        for (const tag of navigator.languages) {
            let lang = tag.split('-')[0];

            // Set first supported locale and break
            if (messages.hasOwnProperty(lang)) {
                initialLocale = lang;
                break;
            }
        }
    } catch {
        console.warn("Error while trying to set preferred locale.")
    }
}

// Create i18n with initialLocale
console.log("Initial locale:", initialLocale);
const i18n = createI18n({
    legacy: false,
    locale: initialLocale,
    fallbackLocale: 'de',
    messages
});

// Set lang attribute of html document to initialLocale
document.documentElement.setAttribute("lang", initialLocale);
// *******************************************************

// Add plugins
app.use(i18n);
app.use(pinia);
app.use(router);

// Start app
app.mount('#app');

// Create umbrella store here to init the WebSocket
const umbrellaStore = useUmbrellaStore();
