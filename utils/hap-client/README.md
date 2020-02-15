# hap-client
> Connect to and control HomeKit devices from Node

[![NPM Version][npm-image]][npm-url]
[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE.md)
![Dependency status](https://david-dm.org/forty2/hap-client.svg)

hap-client is a Node.js module for controlling HomeKit-enabled devices.  It's the client-side counterpart to HomeKit server emulators such as [HAP-NodeJS](https://github.com/KhaosT/HAP-NodeJS/).

## Installation

This module is distributed through NPM:

```sh
npm install hap-client --save

# or, if you prefer:
yarn add hap-client
```

## Examples

This module currently only implements the pairing and control aspects of HomeKit; it doesn't do any device discovery; this may be added in a future release, but you can also discover devices on your local network with an mDNS browser looking for `_hap._tcp` devices:

```sh
# On Linux, for example:
$ avahi-browse -r _hap._tcp
```

Once you know the IP address and port number of your device, you can create a client:

```javascript
import HapClient from 'hap-client';

const client = new HapClient('My Client Name', ip, port);
```

In general, the easiest way to pair with a new device is to use `hap-client-pair` from the [hap-client-tools](https://npmjs.org/package/hap-client-tools/) package.  However, if you want to implement it yourself, you can use the `pair()` method:

```javascript
import HapClient from 'hap-client';

const client = new HapClient('My Client Name', ip, port);
client
  .pair(
      Observable.of('123-45-678')
  )
  .subscribe({
      complete() {
          console.log("Pairing complete");
      }
  });
```

`pair()` takes an [Observable](https://github.com/tc39/proposal-observable) that emits the connected device's PIN code when subscribed to.  It returns a cold Observable that emits nothing and either completes on success or errors on failure.  Note that because the Observable is cold, no work will be done until you call `subscribe()`.  You can also convert the Observable to a Promise by calling `toPromise()`; this will also cause the pairing process to start since Promises are always hot.

Once you've paired with a device, you can use `verifyPairing()` to confirm that the device and client are still paired together.  The need for this should be very rare; it will be done for you automatically as necessary.  Like `pair()`, `verifyPairing()` returns a cold Observable that either completes or errors.

The remaining methods are for communicating with and controlling the device:

### `listAccessories()`

`listAccessories()` returns an Observable that emits a JSON object describing the accessories, services, and characteristics made available by the paired device.  For example, this:

```javascript
client
  .listAccessories()
  .subscribe(
      data => console.log(JSON.stringify(data))
  );
```

might emit:

```json
{
    "accessories": [
        {
            "aid": 1,
            "services": [
                {
                    "iid": 1,
                    "type": "0000003E-0000-1000-8000-0026BB765291",
                    "characteristics": [
                        {
                            "iid": 2,
                            "type": "00000014-0000-1000-8000-0026BB765291",
                            "perms": [
                                "pw"
                            ],
                            "format": "bool",
                            "description": "Identify"
                        }
                    ]
                }
            ]
        }
    ]
}
```

### `getCharacteristics(aid, iid, aid, iid, ...)`

Given a set of accessory ID and instance ID pairs, returns an Observable that emits a JSON object describing the current values of those characteristics.  For example, this:

```javascript
client
  .getAccessories(1, 10, 1, 11)
  .subscribe(
      data => console.log(JSON.stringify(data))
  );
```

might emit (formatted):

```json
{
    "characteristics": [
        {
            "aid": 1,
            "iid": 10,
            "value": 1
        },
        {
            "aid": 1,
            "iid": 11,
            "value": false
        }
    ]
}
```

### `setCharacteristics({ aid, iid, value }, ...)`

`setCharacteristics()` has a sort of dual personality.  On the one hand, it can be used to set the value of characteristics given a set of objects containing accessory ID, instance ID, and value, like this:

```javascript
client
  .setAccessories({ aid: 1, iid: 11, value: true })
  .subscribe();
```

On the other hand, for characteristics that support event notification, `setCharacteristics()` is used to subscribe to the events:

```javascript
client
  .setAccessories({ aid: 1, iid: 11, ev: true })
  .subscribe();
```

Returns an Observable that emits whatever data (if any) is returned by the server; this is implementation dependent and will differ between devices.

### messages

The `messages` property is an Observable that emits each incoming message from the underlying HTTP client.  This includes both messages received in response to a request as well as events.  The messages are in "raw" format, meaning an object with status, headers, and body. For example, this:

```javascript
const client = new HapClient(args.client, args.accessory, args.port);
client
    .messages
    .subscribe(
        res => {
            console.log(
                JSON.stringify(res, null, 4)
            );
        }
    );

client
    .listAccessories()
    .subscribe(
        () => { },
        e => {
            console.error(e)
            client.close();
        },
        () => {
            client.close();
        }
    )
```

might emit this:
```json
{
    "type": "HTTP/1.1",
    "status": "200",
    "statusText": "OK",
    "headers": {
        "content-type": "application/hap+json",
        "date": "Mon, 15 May 2017 01:05:01 GMT",
        "connection": "keep-alive",
        "transfer-encoding": "chunked"
    },
    "body": {
        "accessories": [
            {
                "aid": 1,
                "services": [
                    {
                        "iid": 1,
                        "type": "0000003E-0000-1000-8000-0026BB765291",
                        "characteristics": [
                            {
                                "iid": 2,
                                "type": "00000014-0000-1000-8000-0026BB765291",
                                "perms": [
                                    "pw"
                                ],
                                "format": "bool",
                                "description": "Identify"
                            }
                        ]
                    }
                ]
            }
        ]
    }
}
```

An empty event (which can happen) would look like this:
```json
{
    "type": "EVENT/1.0",
    "status": "200",
    "statusText": "OK",
    "headers": {
        "content-type": "application/hap+json",
        "content-length": "22"
    },
    "body": {
        "characteristics": []
    }
}
```

## Contributing

Contributions are of course always welcome.  If you find problems, please report them in the [Issue Tracker](http://www.github.com/forty2/hap-client/issues/).  If you've made an improvement, open a [pull request](http://www.github.com/forty2/hap-client/pulls).

Getting set up for development is very easy:
```sh
git clone <your fork>
cd hap-client
yarn
```

And the development workflow is likewise straightforward:
```sh
# make a change to the src/ file, then...
yarn build

# or if you want to clean up all the leftover build products:
yarn run clean
```

## Release History

* 1.0.0
    * The first release.

## Meta

Zach Bean – zb@forty2.com

Distributed under the MIT license. See [LICENSE](LICENSE.md) for more detail.

[npm-image]: https://img.shields.io/npm/v/hap-client.svg?style=flat
[npm-url]: https://npmjs.org/package/hap-client
