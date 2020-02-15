# Config

Get the current config

**URL** : `/api/config`

**Method** : `GET`

**Auth required** : YES


## Success Response

**Code** : `200 OK`

**Content example**

```json
{
  "accessory": {
    "hostname": "esp32",
    "pincode": "031-45-712"
  },
  "homekit": {
    "loglevel": 5
  },
  "plugins": {
    "BME280": {
      "enabled": true,
      "interval": 2000
    },
    "DHT": {
      "enabled": true,
      "interval": 5000
    },
    "InfluxDB": {
      "database": "<database>",
      "enabled": true,
      "hostname": "<hostname>",
      "interval": 10000,
      "password": "<password>",
      "port": 8086,
      "username": "<username>"
    },
    "RCSwitch": {
      "enabled": true,
      "interval": 1000,
      "devices": []
    }
  },
  "update": {
    "ota": {
      "enabled": true
    },
    "web": {
      "enabled": false
    }
  },
  "webserver": {
    "enabled": true
  },
  "wifi": {
    "mode": "STA",
    "password": "<password>",
    "ssid": "<ssid>"
  }
}
```

## Error Response

**Condition** : If 'username' and 'password' combination is wrong.

**Code** : `401 Forbidden`