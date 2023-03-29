function fetchSettings() {

    const xhr = new XMLHttpRequest();

    // listen for `load` event
    xhr.onload = () => {

        // print JSON response
        if (xhr.status >= 200 && xhr.status < 300) {
            // parse JSON
            const response = JSON.parse(xhr.responseText);
            this.settings = response
            loadSettings(response)
        }
    };

    xhr.open('GET', '/settings');
    xhr.send()
}

function postSettings() {

    const xhr = new XMLHttpRequest();

    // listen for `load` event
    xhr.onload = () => {

        // print JSON response
        if (xhr.status >= 200 && xhr.status < 300) {
            // parse JSON
            const response = JSON.parse(xhr.responseText);
            this.settings = response
            loadSettings(response)
        }
    };

    xhr.open('POST', '/settings');
    xhr.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
    xhr.send(JSON.stringify(this.settings))
}

function save() {
    const keys = Object.keys(this.settings)
    keys.forEach(function(k) {
        var fields = document.getElementsByName(k);
        if (fields.length > 0) {
            const field = fields[0];
            if (field.type == 'checkbox') {
              this.settings[k] = field.checked
            } else {
              this.settings[k] = parseInt(field.value)
            }
        }

    })

    postSettings();
}

function loadSettings(settings) {
    const keys = Object.keys(settings)
    keys.forEach(function(k) {
        var fields = document.getElementsByName(k);
        if (fields.length > 0) {
            const field = fields[0];
            if (field.type == "checkbox") {
                field.checked = settings[k];
            } else {
                field.value = settings[k];
            }
        }

    })
}

function onLoad() {
    // const timer = setInterval(updateVoltages, 5000);
    fetchSettings();
}
