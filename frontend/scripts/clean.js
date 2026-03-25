PORT = 42690

let isChangingDir = false;
async function changeDir(id) {
    if (isChangingDir) return;
    isChangingDir = true;
    try {
        const res = await fetch('http://localhost:' + PORT + '/change_dir/' + id);
        update();
    } finally {
        setTimeout(() => isChangingDir = false, 250);
    }
}

async function remove(id) {
    const res = await fetch('http://localhost:' + PORT + '/remove/' + id);
    update();
}

let currentOffset = 0;
let isLoadingMore = false;
let hasMoreFiles = true;

function update() {
    currentOffset = 0;
    hasMoreFiles = true;
    let filesContainer = document.getElementById("files-container");
    if (filesContainer) filesContainer.innerHTML = "";
    loadMoreFiles();
}

function loadMoreFiles() {
    if (isLoadingMore || !hasMoreFiles) return;
    isLoadingMore = true;

    const bottomSpinner = document.getElementById("loading-more-spinner");
    if (bottomSpinner) bottomSpinner.style.display = "flex";

    fetch('http://localhost:' + PORT + '/get_interface_data/' + currentOffset)
        .then(response => response.json())
        .then(res => {
            if (currentOffset === 0) {
                document.getElementById("pwd").innerHTML = res["current_path"];
                document.getElementById("removed-label").innerHTML = "Removed " + res["removed_label"];
                document.getElementById("this-info").innerHTML = res["dir_info"];

                removed_percent = res["removed_percent"]
                document.getElementById("removed-bar").style = `width:${removed_percent}%;`;
                document.getElementById("removed-foot").innerHTML = removed_percent + "%" + " cleaned";

                let deletedContainer = document.getElementById("deleted-container");
                if (res["last_removed"]) {
                    deletedContainer.innerHTML = "";
                    res["last_removed"].forEach(item => {
                        let deletedItem = document.createElement("div");
                        deletedItem.className = "deleted-item";
                        deletedItem.innerHTML = `
                        <img src="resources/${item.type}.png" class="file-icon">
                        <div class="deleted-item-name">${item.path}  </div>
                        <div class="deleted-item-size">  ${item.size_label}</div>
                    `;
                        deletedContainer.appendChild(deletedItem);
                    });
                }
            }

            let filesContainer = document.getElementById("files-container");
            res["files"].forEach(file => {
                let item = document.createElement("div");
                item.id = file.id;
                item.className = "file-item";
                item.tabIndex = "0";

                item.innerHTML = `
                <img src="resources/${file.type}.png" class="file-icon">
                <div class="file-title">${file.name}</div>
                <div class="file-size">${file.size}</div>
                <div class="file-date">${file.date_label}</div>
            `;

                item.addEventListener('mousedown', (e) => {
                    if (e.button === 0) {
                        if (document.activeElement === item) {
                            changeDir(item.id);
                        }
                    }
                });
                filesContainer.appendChild(item);
            });

            currentOffset += res.files.length;
            if (currentOffset >= res.dir_size || res.files.length === 0) {
                hasMoreFiles = false;
            }

            if (currentOffset === res.files.length) {
                const loadingScreen = document.getElementById("loading-screen");
                const appContainer = document.getElementById("app-container");
                if (loadingScreen) {
                    loadingScreen.style.opacity = '0';
                    loadingScreen.style.visibility = 'hidden';
                    setTimeout(() => loadingScreen.remove(), 400);
                }
                if (appContainer) {
                    appContainer.style.opacity = '1';
                    appContainer.style.pointerEvents = 'auto';
                }
            }
            
            if (bottomSpinner) bottomSpinner.style.display = "none";
            isLoadingMore = false;
        }).catch(err => {
            console.error(err);
            if (bottomSpinner) bottomSpinner.style.display = "none";
            isLoadingMore = false;
        });
}

window.onload = () => {
    update();

    const explorerContent = document.querySelector('.explorer-content');
    if (explorerContent) {
        explorerContent.addEventListener('scroll', () => {
            if (explorerContent.scrollHeight - explorerContent.scrollTop - explorerContent.clientHeight < 1500) {
                loadMoreFiles();
            }
        });
    }
    const upItem = document.getElementById("up-item");
    upItem.addEventListener('mousedown', (e) => {
        if (e.button === 0) {
            if (document.activeElement === upItem) {
                changeDir("up-item");
            }
        }
    });

    // Bind the top navigation back button to the same action
    const goUpBtn = document.getElementById("go-up");
    if (goUpBtn) {
        goUpBtn.addEventListener('click', () => {
            changeDir("up-item");
        });
    }

    const openThisBtn = document.getElementById("open-this");
    if (openThisBtn) {
        openThisBtn.addEventListener('click', async () => {
            try {
                await fetch('http://localhost:' + PORT + '/open');
            } catch (err) {
                console.error("Failed to open directory:", err);
            }
        });
    }
}

document.addEventListener('keydown', (event) => {
    if (event.key === 'Delete') { // check for Delete key
        const focused = document.activeElement; // get the currently focused element
        if (focused) {
            remove(focused.id);
        }
    }
});

// Capture mouse back button
document.addEventListener('mousedown', (event) => {
    if (event.button === 3 || event.button === 4) {
        event.preventDefault();
    }
});

document.addEventListener('mouseup', (event) => {
    if (event.button === 3) { // 3 is the standard "Browser Back" mouse button
        event.preventDefault();
        changeDir("up-item");
    }
});