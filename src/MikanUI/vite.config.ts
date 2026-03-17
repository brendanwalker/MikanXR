import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import path from 'path'
import fs from 'fs'

// Plugin to clean old hashed files before build
function cleanOldBuildFiles() {
  return {
    name: 'clean-old-build-files',
    buildStart() {
      const jsDir = path.resolve(__dirname, 'resources/js')
      const cssDir = path.resolve(__dirname, 'resources/css')

      // Clean js directory
      if (fs.existsSync(jsDir)) {
        fs.rmSync(jsDir, { recursive: true, force: true })
        console.log('Cleaned resources/js')
      }

      // Clean only generated CSS files (with hashes), keep style.css
      if (fs.existsSync(cssDir)) {
        const files = fs.readdirSync(cssDir)
        files.forEach(file => {
          // Only delete files with hash pattern (e.g., main-DVmuwS01.css)
          if (file !== 'style.css' && file.match(/.*-[a-zA-Z0-9]{8}\.css$/)) {
            fs.unlinkSync(path.join(cssDir, file))
            console.log(`Cleaned ${file}`)
          }
        })
      }
    }
  }
}

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [vue(), cleanOldBuildFiles()],
  resolve: {
    alias: {
      '@': path.resolve(__dirname, './src/ts'),
    },
  },
  base: './', // Use relative paths for local file:// protocol
  build: {
    outDir: 'resources',
    emptyOutDir: false, // Don't delete existing CSS files
    sourcemap: true, // Enable source maps for debugging
    rollupOptions: {
      input: {
        main: path.resolve(__dirname, 'index.html'),
      },
      output: {
        entryFileNames: 'js/[name]-[hash].js', // Add hash for cache busting
        chunkFileNames: 'js/[name]-[hash].js',
        assetFileNames: (assetInfo) => {
          if (assetInfo.name?.endsWith('.css')) {
            return 'css/[name]-[hash][extname]' // Add hash for cache busting
          }
          return 'assets/[name]-[hash][extname]'
        },
      },
    },
  },
  server: {
    port: 3000,
  },
})
