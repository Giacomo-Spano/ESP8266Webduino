var serverPath = '..';

var heaterStatusPath = serverPath+'/heaterstatus';
var temperatureSensorsStatusPath = serverPath+'/temperaturesensorstatus';
var sensorsStatusPath = serverPath+'/sensorstatus';
var commandPath = serverPath+'/command';
var settingsPath = serverPath+'/settings';


var popFunction = function pop(modalText,okCallback,cancelCallback) {

    document.getElementById('modalPopup').style.display = 'block';
    document.getElementById('modalText').innerHTML = modalText;

    var okButton = document.getElementById('okButton');
    var cancelButton = document.getElementById('cancelButton');
    if (okCallback != null) {
        document.getElementById('okButton').style.display = 'block';
        document.getElementById('okButton').onclick = okCallback;
    } else {
        document.getElementById('okButton').style.display = 'none';
    }

    var span = document.getElementsByClassName("close")[0];
    if (cancelCallback != null) {
        document.getElementById('cancelButton').style.display = 'block';
        document.getElementById('cancelButton').onclick = cancelCallback;

        /* When the user clicks on <span> (x), close the modal*/
        span.style.display = 'block';
        span.onclick = function() {
            hideFunction();
        };
        /* When the user clicks anywhere outside of the modal, close it*/
        window.onclick = function(event) {
            if (event.target == modal) {
                hideFunction();
            }
        };

    } else {
        span.style.display = 'none';
        document.getElementById('cancelButton').style.display = 'none';
    }
};

var hideFunction = function hide() {
    document.getElementById('modalPopup').style.display = 'none';
};

/*To detect escape button*/
document.onkeydown = function(evt) {
    evt = evt || window.event;
    if (evt.keyCode == 27) {
        hideFunction('popDiv');
    }
};

function getJson(path, callback) {
    popFunction('Aggiornamento...');

    var request = new XMLHttpRequest();
    request.open('GET', path, true);
    request.setRequestHeader("X-Requested-With", "XMLHttpRequest");
    request.onload = function () {
        if (this.status >= 200 && this.status < 400) {
            var json = JSON.parse(this.response);
            document.getElementById('summary').innerHTML = 'json received' + JSON.stringify(json);
            callback(json);
        } else {
            alert('errore:'+this.status);

        }
        hideFunction();
    };
    request.send();
}

function formInputToJSON(form) {
    var data = {};
    for (var i = 0, ii = form.length; i < ii; ++i) {
        var input = form[i];
        if (input.name) {

            if (input.type == 'number') {
                data[input.name] = Number(input.value);
            }
            else if (input.type == 'radio' && input.checked) {
                if (input.value == '0')
                    data[input.name] = false;
                else
                    data[input.name] = true;
            }
            else if (input.type == 'checkbox') {
                if (input.checked)
                    data[input.name] = true;
                else
                    data[input.name] = false;
            }
            else {
                data[input.name] = input.value;
            }
        }
    }
    return data;
}

function sendCommand(data, callback) {

    var xhr = new XMLHttpRequest();
    xhr.open('POST', commandPath, true);
    xhr.setRequestHeader('Content-Type', 'application/json; charset=UTF-8');
    xhr.send(JSON.stringify(data));

    document.getElementById('command').innerHTML = 'command sent' + JSON.stringify(data);
    document.getElementById('summary').innerHTML = "";

    xhr.onloadend = function () {
        hideFunction();
        result = xhr.responseText;
        var json = JSON.parse(result);
        document.getElementById('summary').innerHTML = 'comman received' + JSON.stringify(json);
        callback(json);
    };
    popFunction('Invio comando in corso...');
}
