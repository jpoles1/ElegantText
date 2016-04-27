metric = "0" //Should be 0 or 1; 0 for F, 1 for C
loc="10546"
url = "http://rss.accuweather.com/rss/liveweather_rss.asp\?metric\="+metric+"\&locCode\="+loc;
var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};
function parseCondition(cond){
  if(["Sunny", "Mostly Sunny", "Partly Sunny", "Intermittent Clouds", "Hazy Sunshine", "Hot"].indexOf(cond) > -1){
    sun = "";
    return sun;
  }
  if(["Mostly Cloudy", "Cloudy", "Dreary (Overcast)", "Fog"].indexOf(cond) > -1){
    cloud = "";
    return cloud;
  }
  if(["Showers", "Mostly Cloudy w/ Showers", "Partly Sunny w/ Showers", "T-Storms", "Mostly Cloudy w/ T-Storms", "Partly Sunny w/ T-Storms", "Rain", "Partly Cloudy w/ Showers", "Partly Cloudy w/ T-Storms"].indexOf(cond) > -1){
    rain = "";
    return rain;
  }
  if(["Windy"].indexOf(cond) > -1){
    wind = "";
    return wind;
  }
  if(["Flurries", "Mostly Cloudy w/ Flurries", "Partly Sunny w/ Flurries", "Snow", "Mostly Cloudy w/ Snow", "Ice", "Sleet", "Freezing Rain", "Rain and Snow", "Cold"].indexOf(cond) > -1){
    snow = "";
    return snow;
  }
  if(["Clear", "Mostly Clear", "Partly Cloudy", "Hazy Moonlight"].indexOf(cond) > -1){
    night = "";
    return night;
  }
}
function getWeather() {
  xhrRequest(url, 'GET',
    function(resp) {
      //USE REGEX TO PULL OUT THE IMPORTANT STUFF
      var current_re = /<title>Currently: ([\w\s]+): (\d{1,3}[FC])<\/title>/
      var weather = resp.match(current_re)
      var dict = {};
      dict['msg_type'] = 1;
      dict['conditions'] = parseCondition(weather[1]);
      dict['temp'] = weather[2];
      // Send to watchapp
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
  dict['msg_type'] = 0;
  dict['bg_R'] = parseInt(backgroundColor.substring(2, 4), 16);
  dict['bg_G'] = parseInt(backgroundColor.substring(4, 6), 16);
  dict['bg_B'] = parseInt(backgroundColor.substring(6), 16);
  dict['txt_R'] = parseInt(textColor.substring(2, 4), 16);
  dict['txt_G'] = parseInt(textColor.substring(4, 6), 16);
  dict['txt_B'] = parseInt(textColor.substring(6), 16);
  dict['ac_R'] = parseInt(accentColor.substring(2, 4), 16);
  dict['ac_G'] = parseInt(accentColor.substring(4, 6), 16);
  dict['ac_B'] = parseInt(accentColor.substring(6), 16);
  dict['blt_vibrate'] = configData['blt_vibrate']
  dict['blt_indicator'] = configData['blt_indicator']


  // Send to watchapp
  Pebble.sendAppMessage(dict, function() {
    console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    console.log('Send failed!');
  });
});
