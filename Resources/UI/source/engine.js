function CallEvent(name, args = []) {
    window.chrome.webview.postMessage({ name, args });
}

function CallResponseEvent(name, args = []) {
    return new Promise((resolve) => {
        function Handler(event) {
            window.chrome.webview.removeEventListener("message", Handler);

            resolve(event.data);
        }

        window.chrome.webview.addEventListener("message", Handler);

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