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
    faXmark,
    faPlus,
    faCheck,
    faCaretDown,
    faCaretUp
} from '@fortawesome/free-solid-svg-icons';

library.add(faPlay, faRotateLeft, faRotateRight, faXmark, faPlus, faCheck, faCaretDown, faCaretUp);

// Setup app
const app = createApp(App);
app.component('Icon', FontAwesomeIcon);
const pinia = createPinia();
app.use(pinia);
app.use(router);

// Start app
app.mount('#app');

// Create umbrella store here to init the WebSocket
const store = useUmbrellaStore();
