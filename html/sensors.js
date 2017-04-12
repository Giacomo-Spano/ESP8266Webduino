var sensorDiv;
var listDiv;
var temperatureDiv;
var doorDiv;
var genericSensorDiv;

var refreshFunction = function refresh(json) {

    listDiv.innerHTML = '';

    for (j = 0; j < json.sensors.length; j++) {
        div = addSensor(json.sensors[j]);
    }

    document.getElementById('addSensorButton').addEventListener("click", function () {
        addSensor(null);
    }, false);

    document.getElementById('saveButton').addEventListener("click", function () {
        save();
    }, false);

    sensorDiv.style.display = 'none';
    temperatureDiv.style.display = 'none';
    doorDiv.style.display = 'none';
    genericSensorDiv.style.display = 'none';
};

function load() {

    sensorDiv = document.getElementById('sensor');

    genericSensorDiv = document.getElementById('genericsensor');
    temperatureDiv = document.getElementById('onewiresensor');
    doorDiv = document.getElementById('doorsensor');

    listDiv = document.getElementById('sensorList');


    getJson(sensorsStatusPath, refreshFunction);
}

function addSensor(sensor) {

    var newSensor = sensorDiv.cloneNode(true); // true means clone all childNodes and all event handlers
    newSensor.style.display = 'block';
    newSensor.getElementsByTagName('input')['deleteButton'].addEventListener("click", function () {
        deleteSensor(newSensor);
    }, false);

    if (sensor != null) {
        newSensor.getElementsByTagName('h2')[0].innerHTML = 'Sensore ' + sensor.name;
        newSensor.getElementsByTagName('input')['enabled'].checked = true;
        newSensor.getElementsByTagName('input')['name'].value = sensor.name;
        newSensor.getElementsByTagName('select')['sensorTypeSelect'].value = sensor.type;
        newSensor.getElementsByTagName('select')['pin'].value = sensor.pin;
    }
    newSensor.getElementsByTagName('select')['sensorTypeSelect'].addEventListener("change", function () {
        onChangeSensorType(this);
    }, false);

    div = newSensor.getElementsByTagName('div')['sensorproperties'];
    if (sensor != null) {
        addSensorProperties(div, sensor.type, sensor);
    } else {
        addSensorProperties(div, "onewiresensor", null);
    }

    listDiv.appendChild(newSensor);
    return newSensor;
}

function addSensorProperties(div,type,sensor) {

    if (type == 'onewiresensor') {
        addTemperatureProperties(div,sensor);
    } else {
        div.innerHTML = '';
    }
}

function addTemperatureProperties(div,sensor) {

    var newDiv = temperatureDiv.cloneNode(true);
    addSubTemperatureSensor(newDiv,sensor);
    newDiv.style.display = 'block';
    div.innerHTML = '';
    div.appendChild(newDiv);
}

function addSubTemperatureSensor(div,sensor) {
    subSensorDiv = div.getElementsByTagName('div')['subsensor'];
    subSensorList = div.getElementsByTagName('div')['subsensorlist'];
    subSensorList.innerHTML = '';

    var numsensors = 1;

    if (sensor != null) {
        for (i = 0; i < sensor.temperaturesensors.length; i++) {
            newsubdiv = subSensorDiv.cloneNode(true);
            newsubdiv.getElementsByTagName('input')['name'].value = sensor.temperaturesensors[i].name;
            subSensorDiv.style.display = 'block';
            subSensorList.appendChild(newsubdiv);
        }
        numsensors = i;
    } else {
        newsubdiv = subSensorDiv.cloneNode(true);
        newsubdiv.getElementsByTagName('input')['name'].value = "nome";
        subSensorDiv.style.display = 'block';
        subSensorList.appendChild(newsubdiv);
        numsensors = 1;
    }

    var numdiv = div.getElementsByTagName('input')['temperatureSensorNumber'];
    numdiv.addEventListener('input', function () {
        //onChangeTemperatureSensorNumberFunction(this.value);

    });
}

function onChangeSensorType(elem/*, value*/) {

    type = elem.value;
    div = elem.parentNode.getElementsByTagName('div')['sensorproperties'];

    addSensorProperties(div,type,null);
};

function deleteSensor(element) {
    element.parentNode.removeChild(element);
}

function save() {

    list = listDiv.getElementsByClassName('box');
    var sensorsJson = {
        'command': 'updatesensorlist',
        sensors: []
    };
    for (i = 0; i < list.length; i++) {

        var item = list[i];
        type = item.getElementsByTagName('select')['sensortype'].value;
        pin = item.getElementsByTagName('select')['pin'].value;
        name = item.getElementsByTagName('input')['name'].value;
        enabled = item.getElementsByTagName('input')['enabled'].checked;

        var properties = {};
        if (type == 'onewiresensor') {

            properties = item.getElementsByTagName('div')['sensorproperties'];
            subsensorlist = properties.getElementsByClassName('subsensorbox');

            var str = '';
            for(k = 0; k < subsensorlist.length;k++) {

                var name = subsensorlist[k].getElementsByTagName('input')['name'].value;
                //temperaturesensor = {};
                //temperaturesensor ['name'] = subsensorlist[k].getElementsByTagName('input')['name'].value;

                if (k > 0)
                    str += ',';

                str += '{' + "\"name\":\"" + name + "\"}";

                //str += JSON.stringify(temperaturesensor);
            }
            properties ['temperaturesensors'] = '[' + str + ']';

        } else if (type == 'doorsensor') {

            properties  = "";

        } else
        {

        }

        //str = poperties.temperaturesensors.replace('\','');

        sensorsJson.sensors.push({
            'name': name,
            'type': type,
            'enabled': enabled,
            'pin': pin,
            'properties' : properties
        });
    }

    sendCommand(sensorsJson, commandResponse);
}

function commandResponse(json) {
    document.getElementById('command').innerHTML += 'command result' + JSON.stringify(json);
    getJson(sensorsStatusPath, refreshFunction);
}
