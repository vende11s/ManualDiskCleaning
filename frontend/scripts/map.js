PORT=42690;

window.onload = () => {
    const params = new URLSearchParams(window.location.search);
    label = params.get("label");
    used = params.get("used");
    id = params.get("id");
    
    document.getElementById("disk-label").innerHTML=label;
    document.getElementById("disk-used-bar").style=`width:${used}%;`;


    mapButton = document.getElementById("map_start");
    mapButton.addEventListener("click", () => {
        let params = String.fromCharCode(id)
        const map_fast = document.getElementById("map_fast");
        const map_windows = document.getElementById("map_windows");
        const map_small = document.getElementById("map_small");

        if(map_fast.checked){
            params+="1"
        } else params+="0"
        if(map_windows.checked){
            params+="1"
        } else params+="0"
        if(map_small.checked){
            params+="1"
        } else params+="0"

    fetch(`http://localhost:${PORT}/map/${params}`, {
        method: 'GET', // or POST if needed
        keepalive: true
    }).catch(() => {});

    window.location.href = `map_prog.html?id=${id}&used=${used}&label=${label}`;
    });
};