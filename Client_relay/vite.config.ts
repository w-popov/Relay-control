import Inspect from 'vite-plugin-inspect';
import { viteSingleFile } from 'vite-plugin-singlefile';
import { fileURLToPath } from 'url';
import { dirname, resolve } from 'path';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

export default { 
  root: resolve(__dirname, './'), 
  base: './',
   
  build: {
    manifest: true,
    emptyOutDir: true,
    target: 'es2023',
    assetsInlineLimit: 100000000,
    rollupOptions: {
      input: {
        main: './index.html' // Собираем из HTML
      },
      output: {
        manualChunks: undefined,
        inlineDynamicImports: true
      }
    }
    
  },
  server: {
    port: 9090,
    hot: true
  },
  css: {
     preprocessorOptions: {
        scss: {
          silenceDeprecations: [
            'import',
            'mixed-decls',
            'color-functions',
            'global-builtin',
          ],
          quietDeps: true,
        },
     },
  },
  plugins: [
    Inspect(), viteSingleFile()
  ],
  optimizeDeps: {
    include: ['lit']
  }
}