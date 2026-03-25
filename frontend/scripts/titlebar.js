// scripts/titlebar.js
document.addEventListener('DOMContentLoaded', () => {
    // Inject Windows style window controls elegantly
    if (!document.querySelector('.windows-controls')) {
        const controls = document.createElement('div');
        controls.className = 'windows-controls';
        
        // Windows 11 crisp SVG icons
        controls.innerHTML = `
            <button class="win-btn win-minimize" id="btn-minimize" title="Minimize">
                <svg width="10" height="10" viewBox="0 0 10 10"><path fill="currentColor" d="M0 4h10v1H0z"/></svg>
            </button>
            <button class="win-btn win-maximize" id="btn-maximize" title="Maximize">
                <svg width="10" height="10" viewBox="0 0 10 10"><path fill="currentColor" d="M1 1h8v8H1V1zm1 1v6h6V2H2z"/></svg>
            </button>
            <button class="win-btn win-close" id="btn-close" title="Close">
                <svg width="10" height="10" viewBox="0 0 10 10"><path fill="currentColor" d="M1.3.6L5 4.3 8.7.6l.7.7L5.7 5l3.7 3.7-.7.7L5 5.7 1.3 9.4l-.7-.7L4.3 5 .6 1.3l.7-.7z"/></svg>
            </button>
        `;
        document.body.prepend(controls);
    }

    // Bind window API events using the preload context bridge
    document.getElementById('btn-close')?.addEventListener('click', () => {
        if (window.windowAPI) window.windowAPI.close();
    });
    
    document.getElementById('btn-minimize')?.addEventListener('click', () => {
        if (window.windowAPI) window.windowAPI.minimize();
    });
    
    document.getElementById('btn-maximize')?.addEventListener('click', () => {
        if (window.windowAPI) window.windowAPI.maximize();
    });
});
