PORT = 42690

async function changeDir(id){
    const res = await fetch('http://localhost:'+PORT+'/change_dir/'+id);
    update();
}

async function remove(id){
    const res = await fetch('http://localhost:'+PORT+'/remove/'+id);
    update();
}

function update(){
    fetch('http://localhost:'+PORT+'/get_interface_data')
    .then(response => response.json())
    .then(res =>{
        document.getElementById("pwd").innerHTML = res["current_path"];
        document.getElementById("removed-label").innerHTML = "Removed " + res["removed_label"];
        document.getElementById("this-info").innerHTML = res["dir_info"];

        removed_percent = res["removed_percent"]
        document.getElementById("removed-bar").style=`width:${removed_percent}%;`;
        document.getElementById("removed-foot").innerHTML=removed_percent + "%" + " cleaned";

        let filesContainer = document.getElementById("files-container");
        filesContainer.innerHTML = ""
        res["files"].forEach(file => {
            console.log(file)
            let item = document.createElement("div");
            item.id = file.id;
            item.className="file-item";
            item.tabIndex="0";

            item.innerHTML = `
                <img src="resources/${file.type}.png" class="file-icon">
                <div class="file-title">${file.name}</div>
                <div class="file-size">${file.size}</div>
                <div class="file-date">${file.date_label}</div>
            `;

            item.addEventListener('dblclick', () => {
                changeDir(item.id);
            });
            filesContainer.appendChild(item);
        });
    });
}

window.onload = () => {
    update();
    document.getElementById("up-item").addEventListener('dblclick', () => {
        changeDir("up-item");
    });
}

document.addEventListener('keydown', (event) => {
  if (event.key === 'Delete') { // check for Delete key
    const focused = document.activeElement; // get the currently focused element
    if (focused) {
      remove(focused.id);
    }
  }
});