import { defineConfig } from 'vitest/config'

export default defineConfig({
  test: {
    globals: true,
    environment: 'node',
    include: ['tests/**/*.{test,spec}.{js,ts}'],
    exclude: ['node_modules', 'dist', 'build']
  },
  resolve: {
    alias: {
      '@': '/client/src',
      '@shared': '/shared'
    }
  }
})