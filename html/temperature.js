var temperatureSensorsCheckBox;
var pinSelect;

var onChangeTemperatureSensorsEnabledFunction = function onChangeTemperatureSensorsEnabled() {

    var jsonString;
    var enabled = false;
    if (this.checked == true ) {
        enabled = true;
    }
    jsonString = '{"command" : "temperaturesensorsettings", "temperaturesensorsenabled" : ' + enabled + '}';
    var json =  JSON.parse(jsonString);
    sendCommand(json, refreshFunction);
};

var onChangePinSelectFunction = function onChangePinSelect() {

    pin = this.value;
    var jsonString = '{"command" : "temperaturesensorsettings", "temperaturepin" : "' + pin + '"}';
    var json =  JSON.parse(jsonString);
    sendCommand(json, refreshFunction);
};



var refreshFunction = function refresh(json) {

    if (json.temperaturesensorsenabled)
        temperatureSensorsCheckBox.checked = true;
    else
        temperatureSensorsCheckBox.checked = false;

    var parentDiv = document.getElementById('temperatureSensorList');
    parentDiv.innerHTML = '';
    var div = document.getElementById('temperatureSensor');


    if (json.sensors.length == 0) {
        div.style.display = 'none';
    } else {
        updateSensorDiv(0,div, json);
    }
    for (i = 1; i < json.sensors.length; i++) {

        /*if (json.sensors[i].type != 'temperature')
            continue;*/

        var clone = div.cloneNode(true); // true means clone all childNodes and all event handlers
        clone.id = div.id + i;
        updateSensorDiv(i,clone, json);
        parentDiv.appendChild(clone);
    }
    var pinSelectControl = document.getElementById('pinSelect');
    pinSelectControl.value = json.temperaturesensorspin;
};

function load() {

    temperatureSensorsCheckBox = document.getElementById('temperatureSensorsCheckBox');
    temperatureSensorsCheckBox.onchange = onChangeTemperatureSensorsEnabledFunction;
    pinSelect = document.getElementById('pinSelect').onchange = onChangePinSelectFunction;
    getJson(temperatureSensorsStatusPath, refreshFunction);
}

function updateSensorDiv(n,div, json) {
    div.getElementsByTagName('h2')[0].innerHTML = 'Sensore ' + json.sensors[n].name;
    div.getElementsByTagName('input')['name'].value = json.sensors[n].name;
    div.getElementsByTagName('input')['addr'].value = json.sensors[n].addr;
    div.getElementsByTagName('label')['temperature'].innerHTML = json.sensors[n].temperature;


    nameForm = div.getElementsByTagName('form')['nameForm'];
    nameForm.id = 'nameFrom'+n;
    nameForm.onsubmit = function () {
        event.preventDefault();
        sendPost(this, commandResponse);
    };
}


function commandResponse(json) {
    document.getElementById('command').innerHTML += 'command result' + JSON.stringify(json);
    getJson(temperatureSensorsStatusPath, refreshFunction);
}

function sendPost(form, callback) {
    var data = formInputToJSON(form);
    sendCommand(data, callback);
}
