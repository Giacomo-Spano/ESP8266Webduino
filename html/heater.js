var startManualForm;
var manualOffForm;
var stopManualForm;
var counter = 0;

var onChangeHeaterEnabledFunction = function onChangeHeaterEnabled() {

    var jsonString;
    var enabled = false;
    if (this.checked == true ) {
        enabled = true;
    }
    jsonString = '{"command" : "heatersettings", "heaterenabled" : ' + enabled + '}';
    var json =  JSON.parse(jsonString);
    sendCommand(json, refreshFunction);
};

var onChangePinSelectFunction = function onChangePinSelect() {

    pin = this.value;
    var jsonString = '{"command" : "heatersettings", "heaterpin" : "' + pin + '"}';
    var json =  JSON.parse(jsonString);
    sendCommand(json, refreshFunction);
}
;

function load() {

    var heaterEnabledCheckBox = document.getElementById('heaterEnabled').onchange = onChangeHeaterEnabledFunction;
    var pinSelect = document.getElementById('pinSelect').onchange = onChangePinSelectFunction;

    /*document.getElementById('heater').onsubmit = function (event) {
        event.preventDefault();
        sendPost(this, commandResponse);
    };*/
    startManualForm = document.getElementById('startManualForm');
    startManualForm.onsubmit = function () {
        event.preventDefault();
        sendPost(this, commandResponse);
    };
    manualOffForm = document.getElementById('manualOffForm');
    manualOffForm.onsubmit = function () {
        event.preventDefault();
        sendPost(this, commandResponse);
    };
    stopManualForm = document.getElementById('stopManualForm');
    stopManualForm.onsubmit = function () {
        event.preventDefault();
        sendPost(this, commandResponse);
    };
    startManualForm.style.display = 'none';
    manualOffForm.style.display = 'none';
    stopManualForm.style.display = 'none';

    getJson(heaterStatusPath, refreshFunction);

    setInterval(function(){
        document.getElementById('timer').innerHTML = counter++;
        if (counter % 20 == 0)
            getJson(heaterStatusPath, refreshFunction); }, 1000);
}

function commandResponse(json) {
    document.getElementById('command').innerHTML += 'command result' + JSON.stringify(json);
    getJson(heaterStatusPath, refreshFunction);
}

var refreshFunction = function refresh(json) {

    if (json.heaterenabled)
        document.getElementById('heaterEnabled').checked = true;
    else
        document.getElementById('heaterEnabled').checked = false;

    var pinSelectControl = document.getElementById('pinSelect');
    pinSelectControl.value = json.heaterpin;

    if (json.status == 'manual' || json.status == 'manualoff') {
        startManualForm.style.display = 'none';
        manualOffForm.style.display = 'none';
        stopManualForm.style.display = 'block';

    } else {
        startManualForm.style.display = 'block';
        manualOffForm.style.display = 'block';
        stopManualForm.style.display = 'none';
    }
};

function sendPost(form, callback) {
    startManualForm.style.display = 'block';
    manualOffForm.style.display = 'block';
    stopManualForm.style.display = 'block';
    var data = formInputToJSON(form);
    sendCommand(data, callback);
}
