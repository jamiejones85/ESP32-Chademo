/** @brief excutes when page finished loading. Creates tables and chart */
var output, websocket,chartSpeed

function toggleHidden(id) {
	const element = document.getElementById(id);
	if (element.classList.contains('hidden')) {
		element.classList.remove('hidden');

	} else {
		element.classList.add('hidden');

	}

	return false
}
function show(id) {
	const element = document.getElementById(id);
	if (element.classList.contains('hidden')) {
		element.classList.remove('hidden');

	}

}

function hide(id) {
	const element = document.getElementById(id);
	if (!element.classList.contains('hidden')) {
		element.classList.add('hidden');

	}

}
function setValue(id, value) {
	const element = document.getElementById(id);
	element.innerHTML = value
}

function updateText(key, data) {
	if (key == 'chademoState') {
		if (data == 0) {
			setValue('mode', "Waiting");
		} else if (data == 1) {
			setValue('mode', "Sending Params");
		} else if (data == 2) {
			setValue('mode', "Wait for Params");
		} else if (data == 3) {
			setValue('mode', "Giving Permission");
		} else if (data == 4) {
			setValue('mode', "Wait for Confirmation");
		} else if (data == 5) {
			setValue('mode', "Close Contactors");
		} else if (data == 6) {
			setValue('mode', "Precharge");
		} else if (data == 7) {
			setValue('mode', "Running");
		} else if (data == 8) {
			setValue('mode', "Sending Stop");
		} else if (data == 9) {
			setValue('mode', "Wait for Stop");
		} else if (data == 10) {
			setValue('mode', "Open Contactors");
		} else if (data == 11) {
			setValue('mode', "Fault");
		} else if (data == 12) {
			setValue('mode', "Stopped");
		} else if (data == 13) {
			setValue('mode', "Limbo");
		} else {
			setValue('mode', "Unknown");
		}
	} else if (key == 'EVSEThresholdVoltage') {
		setValue('evseThreshold', data);
	}  else if (key == 'EVSEAvailableVoltage') {
		setValue('evseVoltage', data);
	} else if (key == 'EVSEAvailableCurrent') {
		setValue('evseAmps', data)
	} else if (key == 'IN1') {
		setValue('in1', data)
	} else if (key == 'IN2') {
		setValue('in2', data)
	} else if (key == 'OUT1') {
		setValue('out1', data)
	} else if (key == 'OUT2') {
		setValue('out2', data)
	}
}
function initHandlers() {

}


function onLoad() {
	chargerWebSocket("ws://"+ location.host +"/ws");

	// output = document.getElementById("output");
	initGauges();
	// initHandlers();
}

function onOpen(evt) {
	console.log("Socket Connected");
}

function onMessage(json) {
	Object.keys(json).forEach(function(key) {
		updateGauge(key, json[key]);
		updateText(key, json[key]);
	});


 }

 function onError(evt) {
	console.log("Socket Error")
 }

 function doSend(message) {
	websocket.send(message);
 }

 function writeToScreen(message) {
	var pre = document.createElement("p");
	pre.style.wordWrap = "break-word";
	pre.innerHTML = message; output.appendChild(pre);
 }

 function chargerWebSocket(wsUri) {
	websocket = new WebSocket(wsUri);

	websocket.onopen = function(evt) {
	   onOpen(evt)
	};

	websocket.onclose = function(evt) {
		console.log(evt)
	};

	websocket.onmessage = function(evt) {
		onMessage(JSON.parse(evt.data))
	};

	websocket.onerror = function(evt) {
	   onError(evt)
	};
 }

 function fileSelected()
{
}
