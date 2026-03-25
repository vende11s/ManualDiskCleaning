PORT = 42690;

function set_bar(x) {
  document.getElementById("map-progress").style = `width:${x}%;`;
}

window.onload = () => {
  const startTime = Date.now();
  const params = new URLSearchParams(window.location.search);
  label = params.get("label");
  used = params.get("used");
  id = params.get("id");

  document.getElementById("disk-label").innerHTML = label;
  document.getElementById("disk-used-bar").style = `width:${used}%;`;


  const interval = setInterval(() => {
    fetch('http://localhost:' + PORT + '/map_info')
      .then(res => res.text())
      .then(text => {
        console.log(text);

        if (text.trim() === 'done') {
          clearInterval(interval);
          console.log('stopped');
          window.location.href = `clean.html?id=${id}&used=${used}&label=${label}`;
        }

        data = JSON.parse(text)
        set_bar(data["progress"])
        document.getElementById("map-progress-info").innerHTML = data["progress_label"];
        document.getElementById("map-progress-pr").innerHTML = Number(data["progress"]).toFixed(2) + "%";
        
        const elapsedSec = Math.floor((Date.now() - startTime) / 1000);
        document.getElementById("time-elapsed").innerHTML = `Time elapsed: ${elapsedSec}s`;
      })
      .catch(err => console.error('Request failed:', err));
  }, 100); // thats interval
};