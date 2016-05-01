var sun = "";
var cloud = "";
var rain = "";
var wind = "";
var snow = "";
var night = "";
var metric = 0;
var loc = "";
if(localStorage.getItem("location")){
  loc = localStorage.getItem("location");
  console.log("Got location from storage")
  console.log(loc)
}
else{
  loc = "77005";
  console.log("Using default zip code", loc)
}
if(localStorage.getItem("metric")){
  metric = localStorage.getItem("metric");
  console.log("Got metric from storage")
  console.log(metric)
}
else{
  metric = "0";
  console.log("Using default metric code", metric)
}
var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};
function parseCondition(cond){
  console.log("Condtions: ", cond)
  if(["Sunny", "Mostly Sunny", "Partly Sunny", "Intermittent Clouds", "Hazy Sunshine", "Hot"].indexOf(cond) > -1){
    return sun;
  }
  if(["Mostly Cloudy", "Cloudy", "Dreary (Overcast)", "Fog"].indexOf(cond) > -1){
    return cloud;
  }
  if(["Showers", "Mostly Cloudy w/ Showers", "Thunderstorms", "Partly Sunny w/ Showers", "T-Storms", "Mostly Cloudy w/ T-Storms", "Partly Sunny w/ T-Storms", "Rain", "Partly Cloudy w/ Showers", "Partly Cloudy w/ T-Storms"].indexOf(cond) > -1){
    return rain;
  }
  if(["Windy"].indexOf(cond) > -1){
    return wind;
  }
  if(["Flurries", "Mostly Cloudy w/ Flurries", "Partly Sunny w/ Flurries", "Snow", "Mostly Cloudy w/ Snow", "Ice", "Sleet", "Freezing Rain", "Rain and Snow", "Cold"].indexOf(cond) > -1){
    return snow;
  }
  if(["Clear", "Mostly Clear", "Partly Cloudy", "Hazy Moonlight"].indexOf(cond) > -1){
    return night;
  }
  return " ";
}
function getWeather() {
  var url = "http://rss.accuweather.com/rss/liveweather_rss.asp\?metric\="+metric+"\&locCode\="+loc;
  var dict = {};
  console.log("Fetching weather using URL:")
  console.log(url)
  xhrRequest(url, 'GET',
    function(resp) {
      //USE REGEX TO PULL OUT THE IMPORTANT STUFF
      var current_re = /<title>Currently: ([\w\s]+): (\d{1,3}[FC])<\/title>/
      if(current_re.test(resp)){
        var weather = resp.match(current_re)
        dict['msg_type'] = 1;
        dict['conditions'] = parseCondition(weather[1]);
        dict['temp'] = weather[2];
        // Send to watchapp
      }
      else{
        dict['msg_type'] = 1;
        dict['conditions'] = rain;
        dict['temp'] = "Err";
      }
      Pebble.sendAppMessage(dict, function() {
        console.log('Send successful: ' + JSON.stringify(dict));
      }, function() {
        console.log('Send failed!');
      });
    }
  );
}
Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
  getWeather();
});
Pebble.addEventListener('appmessage', function(e) {
  console.log("Got request for weather update!");
  console.log(JSON.stringify(e.payload))
  if(e.payload["0"]){
    metric = e.payload[0];
    localStorage.setItem('metric', metric);
  }
  if(e.payload["1"]){
    loc = e.payload[1];
    console.log("Set location to", loc)
    localStorage.setItem("location", loc);
  }
  getWeather();
});
Pebble.addEventListener('showConfiguration', function() {
  var url = 'http://jpoles1.github.io/ElegantText/';
  //dev url:
  //var url = "http://192.168.1.150:8080/"
  console.log('Showing configuration page: ' + url);

  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));
  console.log('Configuration page returned: ' + JSON.stringify(configData));
  var backgroundColor = configData['background_color'];
  var textColor = configData['text_color'];
  var accentColor = configData['accent_color'];
  var dict = {};
  console.log(accentColor)
  dict['msg_type'] = 0;
  dict['bg_R'] = parseInt(backgroundColor.substring(1, 3), 16);
  dict['bg_G'] = parseInt(backgroundColor.substring(3, 5), 16);
  dict['bg_B'] = parseInt(backgroundColor.substring(5, 7), 16);
  dict['txt_R'] = parseInt(textColor.substring(1, 3), 16);
  dict['txt_G'] = parseInt(textColor.substring(3, 5), 16);
  dict['txt_B'] = parseInt(textColor.substring(5, 7), 16);
  dict['ac_R'] = parseInt(accentColor.substring(1, 3), 16);
  dict['ac_G'] = parseInt(accentColor.substring(3, 5), 16);
  dict['ac_B'] = parseInt(accentColor.substring(5, 7), 16);
  dict['blt_vibrate'] = configData['blt_vibrate'];
  if(configData["location"]){
    console.log(configData["location"])
    loc = configData["location"];
    console.log(loc)
    dict["location"] = configData["location"];
    localStorage.setItem("location", loc);
  }
  else{
    dict["location"] = "00000";
  }
  dict['metric'] = parseInt(configData['metric']);
  metric = dict['metric'];
  localStorage.setItem('metric', metric);
  dict['weather_refresh_rate'] = configData['weather_refresh_rate'];
  dict['weather_swap_rate'] = configData['weather_swap_rate'];
  console.log(JSON.stringify(dict))
  // Send to watchapp
  Pebble.sendAppMessage(dict, function() {
    console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    console.log('Send failed!');
  });
});
