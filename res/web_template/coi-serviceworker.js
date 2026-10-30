if (typeof window === 'undefined') {
    self.addEventListener('install', () => self.skipWaiting());
    self.addEventListener('activate', (event) => event.waitUntil(self.clients.claim()));

    self.addEventListener('fetch', (event) => {
        const request = event.request;
        if (request.cache === 'only-if-cached' && request.mode !== 'same-origin') {
            return;
        }
        event.respondWith(
            fetch(request).then((response) => {
                if (response.status === 0) {
                    return response;
                }
                const headers = new Headers(response.headers);
                headers.set('Cross-Origin-Opener-Policy', 'same-origin');
                headers.set('Cross-Origin-Embedder-Policy', 'require-corp');
                headers.set('Cross-Origin-Resource-Policy', 'cross-origin');
                return new Response(response.body, {
                    status: response.status,
                    statusText: response.statusText,
                    headers,
                });
            })
        );
    });
} else if (window.crossOriginIsolated) {
    sessionStorage.removeItem('coiReloaded');
} else if (window.isSecureContext && navigator.serviceWorker && !sessionStorage.getItem('coiReloaded')) {
    const reload = () => {
        sessionStorage.setItem('coiReloaded', '1');
        window.location.reload();
    };
    navigator.serviceWorker.addEventListener('controllerchange', reload);
    navigator.serviceWorker.register(document.currentScript.src).then(() => {
        if (navigator.serviceWorker.controller) {
            reload();
        }
    }, (error) => console.error('coi-serviceworker registration failed:', error));
}
