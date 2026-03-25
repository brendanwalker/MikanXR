import { createApp } from 'vue'
import { createPinia } from 'pinia'
import App from './App.vue'
import '../assets/common-buttons.css'
// Importing @mikanxr/client automatically triggers EnumRegistration.ts at module
// level, which registers all generated enum types with EnumRegistry.
import '@mikanxr/client'

const app = createApp(App)
const pinia = createPinia()

app.use(pinia)
app.mount('#app')
