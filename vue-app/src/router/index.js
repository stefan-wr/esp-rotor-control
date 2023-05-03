import { createRouter, createWebHistory } from 'vue-router';
import MainView from '../views/MainView.vue';
import SettingsView from '../views/SettingsView.vue';
import MessageView from '../views/MessageView.vue';

const router = createRouter({
    history: createWebHistory(import.meta.env.BASE_URL),
    routes: [
        {
            path: '/',
            name: 'main',
            component: MainView
        },
        {
            path: '/settings',
            name: 'settings',
            component: SettingsView
        },
        {
            path: '/settings',
            name: 'reboot',
            component: MessageView,
            props: {
                message:
                    'RotorControl startet sich jetzt neu. Diese Seite aktualisiert sich in 5 Sekunden.',
                pushTo: 'settings',
                pushAfter: 5000
            }
        },
        {
            path: '/settings',
            name: 'disconnect',
            component: MessageView,
            props: {
                message:
                    'RotorControl trennt sich nun vom Netzwerk und startet sich neu. Die Rotor-Steuerung ist damit inaktiv, bis eine neue Netzwerkverbindung eingerichtet wurde. Sie können dieses Fenster schließen.',
                pushTo: '',
                pushAfter: 0
            }
        },

        // Catch-all route to main page
        {
            path: '/:pathMatch(.*)',
            component: MainView
        }
    ]
});

export default router;
