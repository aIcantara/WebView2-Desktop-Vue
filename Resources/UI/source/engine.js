function CallEvent(name, args = []) {
    window.chrome.webview.postMessage({ name, args });
}

function CallResponseEvent(name, args = []) {
    return new Promise((resolve) => {
        function handler(event) {
            window.chrome.webview.removeEventListener("message", handler);

            resolve(event.data);
        }

        window.chrome.webview.addEventListener("message", handler);

        CallEvent(name, args);
    });
}

export const engine = {
    console: {
        Attach() {
            CallEvent("Console:Attach");
        },

        Print(text) {
            CallEvent("Console:Print", [text]);
        }
    },

    math: {
        Sum(a, b) {
            return CallResponseEvent("Math:Sum", [a, b]);
        }
    }
};