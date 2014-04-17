
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
  var tmp = "http://api.openweathermap.org/data/2.5/weather?" + "lat=" + latitude
                                                           + "&lon=" + longitude;
  console.log(tmp);
  req.open('GET', tmp, true);
  req.onload = function(e) {
    if (req.readyState == 4) {
      if(req.status == 200) {
        console.log(req.responseText);
        response = JSON.parse(req.responseText);
        
        var city = response.name;
        console.log("city:");
        console.log(city);

        var main = response.main;
        var weather = response.weather[0];

        var temperature = Math.round((main.temp - 273.15)*1.8 + 32);
        console.log("temperature:");
        console.log(temperature);

        var type = weather.main;
        console.log("type:");
        console.log(type);

        var icon = iconFromWeatherId(weather.id, weather.icon);
        console.log("icon:");
        console.log(icon);

        Pebble.sendAppMessage({
            "icon":icon,
            "temperature":temperature + "\u00B0",
            "type":type
        });

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
    "type":"N/A",
    "temperature":"N/A"
  });
}

function calendar_data () {
  //
  /*
  var apiKey = 'AIzaSyCJ9u0anGIDIsfPK4yEK7W08ivk4CjCvKs';
   var response;
  var req = new XMLHttpRequest();
  var tmp = "https://www.googleapis.com/calendar/v3/users/me/calendarList?key={AIzaSyCJ9u0anGIDIsfPK4yEK7W08ivk4CjCvKs}";
  //console.log(tmp);
  req.open('GET', tmp, true);
  console.log(req.responseText);
  req.onload = function(e) {
    console.log("########");
    console.log(req.response);
    console.log("########");
  }
  */
}

function sports_data(){

  var api = "y5xxzn9dk7c97ttsuxxh5gcb";



  var req = new XMLHttpRequest();
  var url = "http://sports.espn.go.com/mlb/bottomline/scores";
  
  console.log(url);
  req.open('GET', url);
 
  req.onload = function(e) {
    console.log("<sports>");
    //console.log(req.responseText);
    var full = req.responseText;
    full = full.split("?");
    for(i=0; i<full.length; i++){
      var line = full[i];
      //console.log(line);
      line = line.split("&");
      for(j=0; j<line.length; j++){
        var bit = line[j];
        var tmp = "";
        while(bit != tmp){
          tmp = bit;
          bit = bit.replace("%20"," ");
        }
        tmp = "";
        while(bit != tmp){
          tmp = bit;
          bit = bit.replace("  ", " ");
        }

        var piece = bit.split("=");
        if(piece[0].search("left") == -1){
          continue;
        }else{
          console.log(piece[1]);
        }

       // console.log(bit);

      }
      console.log("");
    }
    console.log("<sports/>");
  }
  req.send(null);
}

var locationOptions = { "timeout": 15000, "maximumAge": 60000 }; 
var initialized = false;


Pebble.addEventListener("ready",
  function(e) {
    console.log("connect!" + e.ready);
    locationWatcher = window.navigator.geolocation.watchPosition(locationSuccess, locationError, locationOptions);
    console.log(e.type);
    calendar_data();
    sports_data();
    initialized = true;
  });

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


Pebble.addEventListener("showConfiguration", function() {
  console.log("showing configuration");
  Pebble.openURL('https://dl.dropboxusercontent.com/u/8174738/configurable.html');
});

Pebble.addEventListener("webviewclosed", function(e) {
  console.log("configuration closed");
  // webview closed
  var options = JSON.parse(decodeURIComponent(e.response));
  console.log("Options = " + JSON.stringify(options));
});
