import { fileURLToPath, URL } from "node:url"

import { defineConfig } from "vite"
import vue from "@vitejs/plugin-vue"
import vueDevTools from "vite-plugin-vue-devtools"

export default defineConfig({
    base: "res://",
    server: {
        port: 5050
    },
    plugins: [
        vue(),
        vueDevTools()
    ],
    resolve: {
        alias: {
            "@": fileURLToPath(new URL("./source", import.meta.url))
        }
    }
});