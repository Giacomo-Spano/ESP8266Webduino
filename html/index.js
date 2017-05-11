var shieldId = 0;

var onResetFunction = function onReset() {

    var jsonString = '{"command" : "reset", "shieldid":' + shieldId + '}';
    var json =  JSON.parse(jsonString);
    sendCommand(json, refreshFunction);
}

var onRebootFunction = function onReset() {

    var jsonString = '{"command" : "reboot", "shieldid":' + shieldId + '}';
    var json =  JSON.parse(jsonString);
    sendCommand(json, refreshFunction);
}

var onUpdateSensorStatusFunction = function onReset() {

    var jsonString = '{"command" : "updatesensorstatus", "shieldid":' + shieldId + '}';
    var json =  JSON.parse(jsonString);
    sendCommand(json, refreshFunction);
}

var onRegisterFunction = function onRegister() {

    var jsonString;
    var enabled = false;
    if (this.checked == true ) {
        enabled = true;
    }
    jsonString = '{"command" : "register", "shieldid":' + shieldId + '}';
    var json =  JSON.parse(jsonString);
    sendCommand(json, refreshFunction);
}

function load() {

    shieldId = getUrlVars()["id"];

    var powerCheckBox = document.getElementById('powerCheckBox').onclick = onClickPowerCheckBoxFunction;
    var resetButton = document.getElementById('resetButton').onclick = onResetFunction;
    var rebootButton = document.getElementById('rebootButton').onclick = onRebootFunction;
    var updateStatusButton = document.getElementById('updateStatusButton').onclick = onUpdateSensorStatusFunction;
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
        shieldId = json.shieldid;
        document.getElementById('shieldid').innerHTML = shieldId;
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
        jsonString = '{"command" : "power", "status" : "on", "shieldid":' + shieldId + '}';
    } else {
        alert( "Spengere il dispositivo?" );
        var jsonString = '{"command" : "power", "status" : "off", "shieldid":' + shieldId + '}';
    }

    var json =  JSON.parse(jsonString);
    sendCommand(json, refreshFunction);

}
