import { createRouter, createWebHistory } from 'vue-router';
import MainView from '../views/MainView.vue';
import SettingsView from '../views/SettingsView.vue';
import MessageView from '../views/MessageView.vue';
import UpdateView from '../views/UpdateView.vue';
import LicenseView from '../views/LicenseView.vue';  

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
                message: 'messages.reboot',
                pushTo: 'settings',
                pushAfter: 5000,
                disableHeaderLinks: true
            }
        },
        {
            path: '/settings',
            name: 'reboot-failed',
            component: MessageView,
            props: {
                message: 'messages.rebootFailed',
                pushTo: 'settings',
                pushAfter: 5000
            }
        },
        {
            path: '/settings',
            name: 'disconnect',
            component: MessageView,
            props: {
                message: 'messages.disconnect',
                disableHeaderLinks: true
            }
        },
        {
            path: '/settings',
            name: 'disconnect-failed',
            component: MessageView,
            props: {
                message: 'disconnectFailed',
                pushTo: 'settings',
                pushAfter: 5000
            }
        },
        {
            path: '/update',
            name: 'update',
            component: UpdateView
        },
        {
            path: '/licenses',
            name: 'licenses',
            component: LicenseView

        },

        // Catch-all route to main page
        {
            path: '/:pathMatch(.*)',
            component: MainView
        }
    ]
});

export default router;
