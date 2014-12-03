var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  // build url
  var url = "http://api.openweathermap.org/data/2.5/weather?lat=" + pos.coords.latitude + "&lon=" + pos.coords.longitude + "&units=imperial";
  
  // make our call
  xhrRequest(url, 'GET', function(responseText) {
    var json = JSON.parse(responseText);
    console.log("JSON response " + json.main.temp);
    
    var temperature = Math.round(json.main.temp);
    console.log("Temperature is " + temperature);
    
    var conditions = json.weather[0].main;
    console.log("Conditions are " + conditions);
    
    var dict = {
      "KEY_TEMPERATURE": temperature,
      "KEY_CONDITIONS": conditions
    };
    
    // Send the info to the watch
    Pebble.sendAppMessage(dict,
      function(e) {
        console.log("Weather info sent to pebble successfully");
      },
      function(e) {
        console.log("Error sending data to watch");
      }
      );
  });
}

function locationError(err) {
  console.log("Error requesting location");
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
  locationSuccess,
  locationError,
    {timeout:15000, maximumAge:6000}
  );
}


// Listen for watchface open
Pebble.addEventListener('ready', function(e) {
  console.log("PebbleKit JS is ready");
  getWeather();
});

// Listen for appmessage received
Pebble.addEventListener('appmessage', function(e) {
  console.log("AppMessage received!");
});