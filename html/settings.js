
function load() {

    document.getElementById('shield').onsubmit = function (event) {
        event.preventDefault();
        sendPost(this, commandResponse);
    };
    getJson(settingsPath, refreshFunction);
}

function commandResponse(json) {
    getJson(settingsPath, refreshFunction);
}

var refreshFunction = function refresh(json) {
    document.getElementById('summary').innerHTML = JSON.stringify(json);

    document.getElementById('localport').value = json.localport;
    document.getElementById('shieldname').value = json.shieldname;
    document.getElementById('ssid').value = json.ssid;
    document.getElementById('password').value = json.password;
    document.getElementById('servername').value = json.servername;
    document.getElementById('serverport').value = json.serverport;
};

function sendPost(form, callback) {
    var data = formInputToJSON(form);
    sendCommand(data, callback);
}
