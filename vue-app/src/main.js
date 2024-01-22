import { createApp } from 'vue';
import { createPinia } from 'pinia';
import { useUmbrellaStore } from '@/stores/umbrella';
import './assets/main.scss';

import App from './App.vue';
import router from './router';


// FontAwesome icons
import { FontAwesomeIcon } from '@fortawesome/vue-fontawesome';
import { library } from '@fortawesome/fontawesome-svg-core';
import {
    faPlay,
    faRotateRight,
    faRotateLeft,
    faArrowDown,
    faArrowRight,
    faArrowLeft,
    faXmark,
    faPlus,
    faMinus,
    faCheck,
    faGaugeHigh,
    faArrowsSpin,
    faArrowsRotate,
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
    faCode,
    faMicrochip,
    faCirclePlus,
    faFileArrowUp,
    faFileImport,
    faCircleInfo,
    faCircleCheck,
    faSpinner
} from '@fortawesome/free-solid-svg-icons';

library.add(
    faPlay,
    faRotateLeft,
    faRotateRight,
    faArrowDown,
    faArrowRight,
    faArrowLeft,
    faXmark,
    faPlus,
    faMinus,
    faCheck,
    faGaugeHigh,
    faArrowsSpin,
    faArrowsRotate,
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
    faCode,
    faMicrochip,
    faCirclePlus,
    faFileArrowUp,
    faFileImport,
    faCircleInfo,
    faCircleCheck,
    faSpinner
);

// Setup app
const app = createApp(App);

// Add fontawesome
app.component('Icon', FontAwesomeIcon);

// Add Pinia
const pinia = createPinia();
app.use(pinia);

// Add router
app.use(router);

// Start app
app.mount('#app');

// Create umbrella store here to init the WebSocket
const umbrellaStore = useUmbrellaStore();
