import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import path from 'path'

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [vue()],
  resolve: {
    alias: {
      '@': path.resolve(__dirname, './src/ts'),
    },
  },
  base: './', // Use relative paths for local file:// protocol
  build: {
    outDir: 'resources',
    emptyOutDir: false, // Don't delete existing CSS files
    rollupOptions: {
      input: {
        main: path.resolve(__dirname, 'index.html'),
      },
      output: {
        entryFileNames: 'js/[name].js',
        chunkFileNames: 'js/[name].js',
        assetFileNames: (assetInfo) => {
          if (assetInfo.name?.endsWith('.css')) {
            return 'css/[name][extname]'
          }
          return 'assets/[name][extname]'
        },
      },
    },
  },
  server: {
    port: 3000,
  },
})
