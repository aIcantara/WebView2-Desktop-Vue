<script>
import { engine } from "./engine.js"

export default {
    methods: {
        TestPrint() {
            engine.console.Print("Hello from JS");
        }
    },

    mounted() {
        engine.console.Attach();

        engine.math.Sum(3, 5).then((result) => engine.console.Print(result));
    }
}
</script>

<template>
    <button @click="TestPrint">Test Console Print</button>
</template>

<style>
body {
    background: green;
}
</style>