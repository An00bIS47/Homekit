import { Observable, Subject } from 'rxjs';
import MessageSocket from 'message-socket';
import BufferReader from 'buffer-reader';

import {
    indexOf,
    mark,
    seekToMark,
    nextLine,
    remaining
} from './lib/bufferreader';

import tlv from './lib/tlv';

import { splitGen as split } from './lib/string';

const debug = require('debug')('hap-client:http');

function readChunk() {
    // read one line
    let size = this::nextLine(), chunk = '';
    if (size) {
        size = parseInt(size, 16);
        debug(`reading ${size} bytes for chunk`);
        if (size > 0) {
            if (size < (this.offset + this.buf.length)) {
                chunk = this.nextBuffer(size);
                this.move(2);
            }
            else {
                chunk = null;
            }
        }

        return [ size, chunk ];
    }

    return null;
}

const Decoders = {
    'application/pairing+tlv8': (buffer) =>
        tlv.decode(buffer.restAll())
    ,
    'application/hap+json': (buffer) => {
        const body = buffer.restAll().toString('utf8');
        return JSON.parse(body)
    }
};

const Encoders = {
    /*
    'application/pairing+tlv8': (buffer) =>
        tlv.decode(buffer.restAll())
    ,
    */
    'application/hap+json': (object) => {
        return Buffer.from(JSON.stringify(object), 'utf8');
    }
};

function runMiddleware(funcName, obj) {
    return this
        ._middleware
        .reduce(
            (acc, next) => {
                if (next && next[funcName]) {
                    acc = next[funcName](acc);
                    if (acc === null) {
                        throw new Error("Middleware failure");
                    }
                }
                return acc;
            },
            obj
        );
}

class EventedHttpClient
{
    constructor(host, port = 80) {
        Object.defineProperty(
            this, '_host', {
                value: host
            }
        );

        Object.defineProperty(
            this, '_port', {
                value: port
            }
        );

        Object.defineProperty(
            this, '_socket', {
                value: new MessageSocket(host, port, ::this._bufferSplitter, null)
            }
        );

        Object.defineProperty(
            this, '_middleware', {
                value: []
            }
        );

        Object.defineProperty(
            this, '_isClosing', {
                value: new Subject()
            }
        );
    }

    get messages() {
        return Observable
            .from(
                this._socket
            )
            .takeUntil(
                this._isClosing
            )
    }

    addMiddleware(obj) {
        this._middleware.push(obj);
    }

    request(method, url, headers, data) {
        return Observable
            .defer(
                () => {
                    debug(`requesting: ${method} ${url}`);

                    let request = {
                        method,
                        url,
                        headers,
                        body: data ? data : Buffer.alloc(0)
                    };

                    request =
                        this
                            ::runMiddleware(
                                'handleRequest',
                                request
                            );

                    let outgoing =
                        Buffer
                            .concat([
                                Buffer.from(
                                    `${request.method.toUpperCase()} ${request.url} HTTP/1.1\r\n` +
                                    `Host: ${this._host}:${this._port}\r\n` +

                                    Object
                                        .keys(request.headers)
                                        .reduce(
                                            (acc, h) =>
                                                acc + `${h}: ${request.headers[h]}\r\n`
                                            , ''
                                        ) +

                                    '\r\n'
                                ),

                                request.body
                            ]);

                    debug("raw request: %s", outgoing.toString('hex'));

                    outgoing =
                        this
                            ::runMiddleware(
                                'handleRawRequest',
                                outgoing
                            );

                    debug("raw request (post middleware): %s", outgoing.toString('hex'));

                    this._socket.send(outgoing);

                    debug("Request sent");

                    return this
                        .messages
                        .filter(x => x.type !== 'EVENT/1.0')
                        .take(1)
                }
            )
    }

    get(url, headers = {}) {
        debug("GETing %s", url);
        return this
            .request('GET', url, headers);
    }

    post(url, buffer, contentType = 'application/json', headers = {}) {
        debug("POSTing to %s: %o", url, buffer);
        return this
            .request('POST', url, {
                ['Content-Type']:   contentType,
                ['Content-Length']: buffer.length,
                ...headers
            }, buffer);
    }

    put(url, data, contentType = 'application/json', headers = {}) {
        debug("PUTing %s: %o", url, data);
        let  encoder;
        if (encoder = Encoders[contentType]) {
            data = encoder(data);
        }

        return this
            .request('PUT', url, {
                ['Content-Type']:   contentType,
                ['Content-Length']: data.length,
                ...headers
            }, data);
    }

    disconnect() {
        this._socket.close();
        this._isClosing.next();
    }

    _bufferSplitter(buf) {
        const processed =
            this
                ::runMiddleware(
                    'handleRawResponse',
                    buf
                );

        if (processed.length == 0) {
            // need more data.
            return [ [], buf ];
        }

        let parsed =
            this._parseMessage(new BufferReader(processed));

        return this
            ::runMiddleware(
                'handleResponse',
                parsed
            );
    }

    _parseMessage(buffer) {
        let messages = [], match;

        // ignore everything until a status line
        let statusRe = /(HTTP|EVENT)\/(\d+\.\d+)\s+(\d{3})\s+(.*?)$/;

        while (buffer::indexOf("\r\n") >= 0) {
            let line = buffer::nextLine();

            if (match = statusRe.exec(line)) {
                let [, messageType, version, status, statusText ] = match;

                debug(`status: ${messageType}, ${version}, ${status}, ${statusText}`);

                let idx = -1, headers = {};
                while ((idx = buffer::indexOf("\r\n")) > 0) {
                    let header = buffer::nextLine();
                    let [name, value] = header::split(/:\s*/, 2);

                    headers[name.toLowerCase()] = value;
                }

                // lose the blank line
                buffer.move(2);

                let body = new BufferReader(new Buffer([]));

                if (status != 204) { // "No Content"
                    // is there a content length header?
                    if (headers['content-length']) {
                        let len = parseInt(headers['content-length']);
                        debug(`Reading ${len} bytes for body...`);
                        debug(`There are ${buffer::remaining()} bytes left in the buffer`);
                        if (buffer::remaining() >= len) {
                            body.append(buffer.nextBuffer(len));
                        } else {
                            // the whole message is not in the buffer
                            // wait till next time
                            debug('partial message; returning');
                            return [ [], buffer.buf ];
                        }
                    } else if (headers['transfer-encoding'].toLowerCase() === 'chunked') {
                        // TODO: read chunked encoding
                        debug(`Reading chunked bytes for body`);

                        let chunkInfo;
                        while (chunkInfo = buffer::readChunk()) {
                            let [ declaredSize, chunk ] = chunkInfo;
                            if (declaredSize) {
                                if (chunk) {
                                    body.append(chunk);
                                    debug('read chunk sized ' + declaredSize);
                                }
                                else {
                                    // the whole message is not in the buffer
                                    // wait till next time
                                    debug('partial message; returning');
                                    return [ [], buffer.buf ];
                                }
                            }
                        }

                        // read trailers
                        while ((idx = buffer::indexOf("\r\n")) > 0) {
                            let header = buffer::nextLine();
                            let [name, value] = header::split(/:\s*/, 2);

                            headers[name.toLowerCase()] = value;
                        }

                        // TODO: I feel like I should need this
                        // buffer.move(2);
                    }
                }

                debug('finished reading');

                let contentType, decoder;
                if ((contentType = headers['content-type'])
                        && (decoder = Decoders[contentType]))
                {
                    debug('parsing body');
                    body = decoder(body);
                }

                if (body instanceof BufferReader && body.buf.length === 0) {
                    body = null;
                }


                messages.push({ type: `${messageType}/${version}`, status, statusText, headers, body })
                buffer::mark();
            }
        }

        buffer::seekToMark();
        debug(`returning from ${buffer.offset} to ${buffer.buf.length}`);

        return [ messages, buffer.restAll() ];
    }
}

export {
    EventedHttpClient as default
}
