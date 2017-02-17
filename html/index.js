var onResetFunction = function onReset() {

    var jsonString = '{"command" : "reset"}';
    var json =  JSON.parse(jsonString);
    sendCommand(json, refreshFunction);
}

var onRegisterFunction = function onRegister() {

    var jsonString;
    var enabled = false;
    if (this.checked == true ) {
        enabled = true;
    }
    jsonString = '{"command" : "register"}';
    var json =  JSON.parse(jsonString);
    sendCommand(json, refreshFunction);
}

function load() {

    var powerCheckBox = document.getElementById('powerCheckBox').onclick = onClickPowerCheckBoxFunction;
    var resetButton = document.getElementById('resetButton').onclick = onResetFunction;
    var registerButton = document.getElementById('registerButton').onclick = onRegisterFunction;

    getJson(settingsPath, refreshFunction);
}

var refreshFunction = function refresh(json) {

    if (json.hasOwnProperty("power")) {
        if (json.power == "on")
            document.getElementById('powerCheckBox').checked = true;
        else
            document.getElementById('powerCheckBox').checked = false;
    }

    if (json.hasOwnProperty("datetime")) {
        document.getElementById('datetime').innerHTML = json.datetime;
    }
    if (json.hasOwnProperty("localip")) {
        document.getElementById('localip').innerHTML = json.localip;
    }
    if (json.hasOwnProperty("macaddress")) {
        document.getElementById('macaddress').innerHTML = json.macaddress;
    }
    if (json.hasOwnProperty("shieldid")) {
        document.getElementById('shieldid').innerHTML = json.shieldid;
    }
    if (json.hasOwnProperty("shieldname")) {
        document.getElementById('shieldname').innerHTML = json.shieldname;
    }
    if (json.hasOwnProperty("lastrestart")) {
        document.getElementById('lastrestart').innerHTML = json.lastrestart;
    }
    if (json.hasOwnProperty("heap")) {
        document.getElementById('heap').innerHTML = json.heap;
    }
    if (json.hasOwnProperty("swversion")) {
        document.getElementById('swversion').innerHTML = json.swversion;
    }
};

function sendPost(form, callback) {
    var data = formInputToJSON(form);
    sendCommand(data, callback);
}

var onClickPowerCheckBoxFunction = function onClickPowerCheckBox() {

    var jsonString;
    if ( this.checked == true ) {
        alert( "Accendere il dispositivo?" );
        jsonString = '{"command" : "power", "status" : "on"}';
    } else {
        alert( "Spengere il dispositivo?" );
        var jsonString = '{"command" : "power", "status" : "off"}';
    }

    var json =  JSON.parse(jsonString);
    sendCommand(json, refreshFunction);

}
