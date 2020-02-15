// Homebridge plugin to reading DHT22 Sensor on a Raspberry PI.  Assumes DHT22
// is connected to GPIO 4 by default.

// Uses pigpio library to access gpio pin, and a custom program dht22 read the sensor.

//"accessories": [{
//    "accessory": "Dht",
//    "name": "cputemp",
//    "service": "Temperature"
//}, {
//    "accessory": "Dht",
//    "name": "Temp/Humidity Sensor",
//    "service": "dht22"
//}, {        // For testing
//    "accessory": "Dht",
//    "name": "Test-DHT",
//    "service": "dht22",
//    "dhtExec": "Code/homebridge-dht/test/dht22"
//}]
//
// or Multiple
//
//"accessories": [{
//    "accessory": "Dht",
//    "name": "cputemp",
//    "service": "Temperature"
//}, {
//    "accessory": "Dht",
//    "name": "Temp/Humidity Sensor - Indoor",
//    "service": "dht22",
//    "gpio": "4"
//}, {
//    "accessory": "Dht",
//    "name": "Temp/Humidity Sensor - Outdoor",
//    "service": "dht22",
//    "gpio": "5"
//}]




var Service, Characteristic, FakeGatoHistoryService;
var exec = require('child_process').execFile;
var cputemp, dhtExec;
var debug = require('debug')('fakeDHT');
var logger = require("mcuiot-logger").logger;
const moment = require('moment');
var os = require("os");
var hostname = os.hostname();

module.exports = function(homebridge) {
  Service = homebridge.hap.Service;
  Characteristic = homebridge.hap.Characteristic;
  CustomCharacteristic = require('./lib/CustomCharacteristic.js')(homebridge);
  FakeGatoHistoryService = require('fakegato-history')(homebridge);
  homebridge.registerAccessory("homebridge-fakedht", "fakeDht", FakeDhtAccessory);
}

function FakeDhtAccessory(log, config) {
  this.log = log;
  this.log("Adding Accessory");
  this.config = config;
  this.name = config.name;
  this.name_temperature = config.name_temperature || config.name;
  this.name_humidity = config.name_humidity || config.name;
  this.service = config.service || "dht22";
  this.gpio = config.gpio || "4";
  this.refresh = config.refresh || "10"; // Every minute
  this.storage = config['storage'] || "fs";

  dhtExec = config.dhtExec || "dht22";
  cputemp = config.cputemp || "cputemp";

  this.log_event_counter = 59;
  this.spreadsheetId = config['spreadsheetId'];
  if (this.spreadsheetId) {
    this.logger = new logger(this.spreadsheetId);
  }


}

FakeDhtAccessory.prototype = {

  getDHTTemperature: function(callback) {
    exec(dhtExec, ['-g', this.gpio], function(error, responseBody, stderr) {
      if (error !== null) {
        this.log('dhtExec function failed: ' + error);
        callback(error);
      } else {
        // dht22 output format - gives a 3 in the first column when it has troubles
        // 0 24.8 C 50.3 %
        var result = responseBody.toString().split(/[ \t]+/);
        var temperature = parseFloat(result[1]);
        var humidity = parseFloat(result[3]);

        //                this.humidity = humidity;
        this.log("DHT Status: %s, Temperature: %s, Humidity: %s", result[0], temperature, humidity);


        this.log_event_counter = this.log_event_counter + 1;
        if (this.log_event_counter > 59) {
          if (this.spreadsheetId) {
            this.logger.storeDHT(this.name, result[0], temperature, humidity);
          }
          this.log_event_counter = 0;
        }

        var err;
        if (parseInt(result[0]) !== 0) {
          this.log.error("Error: dht22 read failed with status %s", result[0]);
          err = new Error("dht22 read failed");
          humidity = err;
        } else {

          this.loggingService.addEntry({
            time: moment().unix(),
            temp: temperature,
            humidity: humidity,
            pressure: 234
          });

        }

        this.dhtService
            .setCharacteristic(Characteristic.CurrentTemperature, temperature);

        this.dhtService
            .setCharacteristic(CustomCharacteristic.AtmosphericPressureLevel, 234);

        this.dhtService
          .setCharacteristic(Characteristic.CurrentRelativeHumidity, humidity);

        
        callback(err, temperature);
      }
    }.bind(this));
  },

  getTemperature: function(callback) {
    exec(cputemp, function(error, responseBody, stderr) {
      if (error !== null) {
        this.log('cputemp function failed: ' + error);
        callback(error);
      } else {
        var binaryState = parseFloat(responseBody);
        this.log("Got Temperature of %s", binaryState);

        callback(null, binaryState);
      }
    }.bind(this));
  },


  identify: function(callback) {
    this.log("Identify requested!");
    callback(); // success
  },

  getServices: function() {

    this.log("INIT: %s", this.name);

    // you can OPTIONALLY create an information service if you wish to override
    // the default values for things like serial number, model, etc.
    var informationService = new Service.AccessoryInformation();

    informationService
      .setCharacteristic(Characteristic.Manufacturer, "fakedht")
      .setCharacteristic(Characteristic.Model, this.service)
      .setCharacteristic(Characteristic.SerialNumber, hostname + "-" + this.name)
      .setCharacteristic(Characteristic.FirmwareRevision, require('./package.json').version);

    switch (this.service) {

      case "Temperature":
        this.temperatureService = new Service.TemperatureSensor(this.name);
        this.temperatureService
          .getCharacteristic(Characteristic.CurrentTemperature)
          .setProps({
            minValue: -100,
            maxValue: 100
          })
          .on('get', this.getTemperature.bind(this));

        setInterval(function() {
          this.getTemperature(function(err, temp) {
            if (err)
              temp = err;
            this.temperatureService
              .getCharacteristic(Characteristic.CurrentTemperature).updateValue(temp);
          }.bind(this));

        }.bind(this), this.refresh * 1000);

        return [informationService, this.temperatureService];
      case "dht22":
        this.dhtService = new Service.TemperatureSensor(this.name_temperature);
        this.dhtService
          .getCharacteristic(Characteristic.CurrentTemperature)
          .setProps({
            minValue: -100,
            maxValue: 100
          });

        // this.humidityService = new Service.HumiditySensor(this.name_humidity);
        this.dhtService.addCharacteristic(CustomCharacteristic.AtmosphericPressureLevel);
        this.dhtService.addCharacteristic(Characteristic.CurrentRelativeHumidity);



        this.dhtService.log = this.log;
        this.loggingService = new FakeGatoHistoryService("weather", this.dhtService, {
          storage: this.storage,
          minutes: this.refresh * 10 / 60
        });

        setInterval(function() {
          this.getDHTTemperature(function(err, temp) {
            if (err)
              temp = err;
            this.dhtService
              .getCharacteristic(Characteristic.CurrentTemperature).updateValue(temp);
          }.bind(this));

        }.bind(this), this.refresh * 1000);

        this.getDHTTemperature(function(err, temp) {
          this.dhtService
            .setCharacteristic(Characteristic.CurrentTemperature, temp);
        }.bind(this));










        return [this.dhtService, informationService, this.loggingService];

    }
  }
};
