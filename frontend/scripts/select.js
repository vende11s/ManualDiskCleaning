PORT=42690;

window.onload = () => {
    gridContainer = document.getElementById("grid-container");
    gridContainer.innerHTML="";

    const loadDisks = () => {
        fetch('http://localhost:'+PORT+"/init_disks")
        .then(response => {
            if (!response.ok) throw new Error("API not ready");
            return response.json();
        })
        .then(data => {
    data["disks"].forEach(disk => {
        item = document.createElement("div");
        item.id = disk.id;
        item.className="grid-item";

        item.innerHTML = `
        <div class="progress-info">
          <span class="label">${disk.label}</span>
          <div class="progress-bar-container">
            <div class="progress-bar-fill" style="width: ${disk.used}%;"></div>
          </div>
        </div>
      `;

      item.addEventListener("click", () => {
        // redirect to another page with the id as a query param
        window.location.href = `map.html?id=${disk.id}&used=${disk.used}&label=${disk.label}`;
      });

      // append it
      gridContainer.appendChild(item);
    });

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

  })
  .catch(error => {
      console.error('Unable to fetch disk data, retrying in 1s...', error);
      setTimeout(loadDisks, 1000);
  });
  };

  loadDisks();
};