//code modified from:
//Pebble SDK/Examples/pebble-kit-js/weather/src...

function iconFromWeatherId(weatherId, iconId) {
  if (weatherId == 801 || weatherId == 802) {
    return 0; // cloudy

  } else if (weatherId > 500 && weatherId < 600) {
    return 1; //heavy rain

  } else if (weatherId == 500 || (weatherId >= 300 && weatherId <=321)) {
    return 2; //light rain

  } else if (weatherId >= 600 && weatherId < 700) {
    return 3; // snow

  } else if ((weatherId >= 200 && weatherId < 300) || weatherId >= 960 && weatherId <= 962) {
    return 4; // lightning

  }  else if (weatherId == 803 || weatherId == 804) {
    return 5; // mostly cloudy

  } else if (weatherId == 904) {
    return 6; //sun

  }else if (weatherId == 800) {
  	if (iconId == "01d"){
  		return 6 // sun
  	}
  	if (iconId == "01n"){
  		return 7 // moon
  	} 
  } else if (weatherId == 905 || (weatherId >= 952 && weatherId <= 959)) {
    return 8; // wind

  } else {
    return 9; // other
  }
}

function fetchWeather(latitude, longitude) {
  var response;
  var req = new XMLHttpRequest();
  req.open('GET', "http://api.openweathermap.org/data/2.1/find/city?" +
    "lat=" + latitude + "&lon=" + longitude + "&cnt=1", true);
  req.onload = function(e) {
    if (req.readyState == 4) {
      if(req.status == 200) {
      	console.log("Raw json data dump:")
        console.log(req.responseText);
        response = JSON.parse(req.responseText);
        var temperature, icon; //,city
        if (response && response.list && response.list.length > 0) {
          var weatherResult = response.list[0];
          //conversion from k to f
          temperature = Math.round((weatherResult.main.temp - 273.15)*1.8 + 32);
          icon = iconFromWeatherId(weatherResult.weather[0].id, weatherResult.weather[0].icon);
          //city = weatherResult.name;
          console.log("Temperature:")
          console.log(temperature);
          console.log("Icon code:")
          console.log(icon);
          //console.log(city);
          Pebble.sendAppMessage({
            "icon":icon,
            "temperature":temperature + "\u00B0F",
            //"city":city
          });
        }

      } else {
        console.log("Error");
      }
    }
  }
  req.send(null);
}

function locationSuccess(pos) {
  var coordinates = pos.coords;
  fetchWeather(coordinates.latitude, coordinates.longitude);
}

function locationError(err) {
  console.warn('location error (' + err.code + '): ' + err.message);
  Pebble.sendAppMessage({
    //"city":"Loc Unavailable",
    "temperature":"N/A"
  });
}

var locationOptions = { "timeout": 15000, "maximumAge": 60000 }; 



Pebble.addEventListener("ready",
    function(e) {
        console.log("JavaScript running...");
        locationWatcher = window.navigator.geolocation.watchPosition(locationSuccess, locationError, locationOptions);
        console.log(e.type);
        
    }
);
Pebble.addEventListener("appmessage",
    function(e) {
    window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
    console.log(e.type);
    console.log(e.payload.temperature);
    console.log("message!");
});

Pebble.addEventListener("webviewclosed",
    function(e) {
    console.log("webview closed");
    console.log(e.type);
    console.log(e.response);
});