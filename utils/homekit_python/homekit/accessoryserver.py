#
# Copyright 2018 Joachim Lusiardi
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import binascii
import hashlib
import io
import json
from json.decoder import JSONDecodeError
import select
import threading

from http.server import HTTPServer, BaseHTTPRequestHandler
from socketserver import ThreadingMixIn

import hkdf
from zeroconf import Zeroconf, ServiceInfo
import socket
import sys
import logging
import ed25519

from cryptography.hazmat.primitives.asymmetric import x25519
from cryptography.hazmat.primitives import serialization

from homekit.crypto.chacha20poly1305 import chacha20_aead_decrypt, chacha20_aead_encrypt
from homekit.crypto.srp import SrpServer

from homekit.exceptions import ConfigurationError, ConfigLoadingError, ConfigSavingError, FormatError, \
    CharacteristicPermissionError, DisconnectedControllerError
from homekit.http_impl import HttpStatusCodes
from homekit.model import Accessories, Categories
from homekit.model.characteristics import CharacteristicsTypes
from homekit.protocol import TLV
from homekit.protocol.statuscodes import HapStatusCodes


class AccessoryServer(ThreadingMixIn, HTTPServer):
    """
    This server makes accessories accessible via the HomeKit protocol.
    """

    def __init__(self, config_file, logger=sys.stderr):
        """
        Create a new server that acts like a homekit accessory. The config file is loaded and checked.

        :param config_file: the file that contains the configuration data. Must be a string representing an absolute
        path to the file
        :param logger: this can be None to disable logging, sys.stderr to use the default behaviour of the python
        implementation or an instance of logging.Logger to use this.
        :raises HomeKitConfigurationException: if the config file is malformed. Reason will be in the message.
        """
        if logger is None or logger == sys.stderr or isinstance(logger, logging.Logger):
            self.logger = logger
        else:
            raise ConfigurationError('Invalid logger given.')

        self.data = AccessoryServerData(config_file)
        self.data.increase_configuration_number()
        self.sessions = {}
        self.zeroconf = Zeroconf()
        self.mdns_type = '_hap._tcp.local.'
        self.mdns_name = self.data.name + '._hap._tcp.local.'
        self.identify_callback = None
        self.zeroconf_info = None

        self.accessories = Accessories()

        HTTPServer.__init__(self, (self.data.ip, self.data.port), AccessoryRequestHandler)

    def write_event(self, characteristics, source=None):
        dead_sessions = []
        for session_id, session in self.sessions.items():
            if source and session_id == source:
                continue
            try:
                session['handler'].write_event(characteristics)
            except DisconnectedControllerError:
                dead_sessions.append(session_id)
        for session_id in dead_sessions:
            del self.sessions[session_id]

    def add_accessory(self, accessory):
        self.accessories.add_accessory(accessory)

    def set_identify_callback(self, func):
        """
        Sets the callback function for this accessory server. This will NOT be applied to all accessories registered
        with. This function will be called on unpaired calls to identify.

        :param func: a function without any parameters and without return type.
        """
        self.identify_callback = func

    def publish_device(self):
        desc = {'md': str(self.data.name),  # model name of accessory
                # category identifier (page 254, 2 means bridge), must be a String
                'ci': str(Categories[self.data.category]),
                'pv': '1.0',  # protocol version
                'c#': str(self.data.configuration_number),
                # configuration (consecutive number, 1 or greater, must be changed on every configuration change)
                'id': self.data.accessory_pairing_id_bytes,  # id MUST look like Mac Address
                'ff': '0',  # feature flags (Table 5-8, page 69)
                's#': '1',  # must be 1
                'sf': '1'  # status flag, lowest bit encodes pairing status, 1 means unpaired
                }
        if self.data.is_paired:
            desc['sf'] = '0'

        self.zeroconf_info = ServiceInfo(self.mdns_type, self.mdns_name, addresses=[socket.inet_aton(self.data.ip)],
                                         port=self.data.port, properties=desc)
        self.zeroconf.unregister_service(self.zeroconf_info)
        self.zeroconf.register_service(self.zeroconf_info, allow_name_change=True)

    def unpublish_device(self):
        if self.zeroconf_info:
            self.zeroconf.unregister_service(self.zeroconf_info)

    def shutdown(self):
        # tell all handlers to close the connection
        for session in self.sessions:
            self.sessions[session]['handler'].close_connection = True
        self.socket.close()
        HTTPServer.shutdown(self)


class AccessoryServerData:
    """
    This class is used to take care of the servers persistence to be able to manage restarts,
    """

    def __init__(self, data_file):
        self.data_file = data_file
        try:
            with open(data_file, 'r') as input_file:
                self.data = json.load(input_file)
        except PermissionError:
            raise ConfigLoadingError('Could not open "{f}" due to missing permissions'.format(f=input_file))
        except JSONDecodeError:
            raise ConfigLoadingError('Cannot parse "{f}" as JSON file'.format(f=input_file))
        except FileNotFoundError:
            raise ConfigLoadingError('Could not open "{f}" because it does not exist'.format(f=input_file))

        self.check()

        # set some default values
        if 'peers' not in self.data:
            self.data['peers'] = {}
        if 'unsuccessful_tries' not in self.data:
            self.data['unsuccessful_tries'] = 0

    def _save_data(self):
        try:
            with open(self.data_file, 'w') as output_file:
                json.dump(self.data, output_file, indent=2, sort_keys=True)
        except PermissionError:
            raise ConfigSavingError('Could not write "{f}" due to missing permissions'.format(f=self.data_file))
        except FileNotFoundError:
            raise ConfigSavingError(
                'Could not write "{f}" because it (or the folder) does not exist'.format(f=self.data_file))

    @property
    def ip(self) -> str:
        return self.data['host_ip']

    @property
    def port(self) -> int:
        return self.data['host_port']

    @property
    def setup_code(self) -> str:
        return self.data['accessory_pin']

    @property
    def accessory_pairing_id_bytes(self) -> bytes:
        return self.data['accessory_pairing_id'].encode()

    @property
    def unsuccessful_tries(self) -> int:
        return self.data['unsuccessful_tries']

    def register_unsuccessful_try(self):
        self.data['unsuccessful_tries'] += 1
        self._save_data()

    @property
    def is_paired(self) -> bool:
        return len(self.data['peers']) > 0

    @property
    def name(self) -> str:
        return self.data['name']

    @property
    def category(self) -> str:
        try:
            category = self.data['category']
        except KeyError:
            raise ConfigurationError('category missing in "{f}"'.format(f=self.data_file))
        if category not in Categories:
            raise ConfigurationError('invalid category "{c}" in "{f}"'.format(c=category, f=self.data_file))
        return category

    def remove_peer(self, pairing_id: bytes):
        del self.data['peers'][pairing_id.decode()]
        self._save_data()

    def set_peer_permissions(self, pairing_id: bytes, admin: bool):
        peer = self.data['peers'][pairing_id.decode()]
        peer['admin'] = admin
        self._save_data()

    def add_peer(self, pairing_id: bytes, ltpk: bytes, admin: bool):
        self.data['peers'][pairing_id.decode()] = {'key': binascii.hexlify(ltpk).decode(), 'admin': admin}
        self._save_data()

    def get_peer_key(self, pairing_id: bytes) -> bytes:
        if pairing_id.decode() in self.data['peers']:
            return bytes.fromhex(self.data['peers'][pairing_id.decode()]['key'])
        else:
            return None

    def is_peer_admin(self, pairing_id: bytes) -> bool:
        return self.data['peers'][pairing_id.decode()]['admin']

    @property
    def peers(self):
        return self.data['peers'].keys()

    @property
    def accessory_ltsk(self) -> bytes:
        if 'accessory_ltsk' in self.data:
            return bytes.fromhex(self.data['accessory_ltsk'])
        else:
            return None

    @property
    def accessory_ltpk(self) -> bytes:
        if 'accessory_ltpk' in self.data:
            return bytes.fromhex(self.data['accessory_ltpk'])
        else:
            return None

    def set_accessory_keys(self, accessory_ltpk: bytes, accessory_ltsk: bytes):
        self.data['accessory_ltpk'] = binascii.hexlify(accessory_ltpk).decode()
        self.data['accessory_ltsk'] = binascii.hexlify(accessory_ltsk).decode()[:64]
        self._save_data()

    @property
    def configuration_number(self) -> int:
        return self.data['c#']

    def increase_configuration_number(self):
        self.data['c#'] += 1
        self._save_data()

    def check(self, paired=False):
        """
        Checks a accessory config file for completeness.
        :param paired: if True, check for keys that must exist after successful pairing as well.
        :return: None, but a HomeKitConfigurationException is raised if a key is missing
        """
        required_fields = ['name', 'host_ip', 'host_port', 'accessory_pairing_id', 'accessory_pin', 'c#', 'category']
        if paired:
            required_fields.extend(['accessory_ltpk', 'accessory_ltsk', 'peers', 'unsuccessful_tries'])
        for f in required_fields:
            if f not in self.data:
                raise ConfigurationError(
                    '"{r}" is missing in the config file "{f}"!'.format(r=f, f=self.data_file))


class AccessoryRequestHandler(BaseHTTPRequestHandler):
    VALID_METHODS = ['GET', 'HEAD', 'POST', 'PUT', 'DELETE', 'CONNECT', 'OPTIONS', 'TRACE']
    DEBUG_PUT_CHARACTERISTICS = False
    DEBUG_CRYPT = False
    DEBUG_PAIR_VERIFY = False
    DEBUG_GET_CHARACTERISTICS = False
    timeout = 300

    def __init__(self, request, client_address, server):
        # keep pycharm from complaining about those not being define in __init__
        self.session_id = '{ip}:{port}'.format(ip=client_address[0], port=client_address[1])
        if self.session_id not in server.sessions:
            server.sessions[self.session_id] = {'handler': self}
        self.rfile = None
        self.wfile = None
        self.body = None
        self.PATHMAPPING = {
            '/accessories': {
                'GET': self._get_accessories
            },
            '/characteristics': {
                'GET': self._get_characteristics,
                'PUT': self._put_characteristics
            },
            '/identify': {
                'POST': self._post_identify
            },
            '/pair-setup': {
                'POST': self._post_pair_setup
            },
            '/pair-verify': {
                'POST': self._post_pair_verify
            },
            '/pairings': {
                'POST': self._post_pairings
            }
        }
        self.protocol_version = 'HTTP/1.1'
        self.close_connection = False

        self.timeout_counter = 0

        # get the identify callback function from calling server
        self.identify_callback = server.identify_callback

        self.write_lock = threading.Lock()
        self.subscriptions = set()

        # init super class
        BaseHTTPRequestHandler.__init__(self, request, client_address, server)

    def setup(self):
        super().setup()

        self.orig_wfile = self.wfile
        self.orig_rfile = self.rfile

    def write_event(self, characteristics):
        tmp = []
        for (aid, iid) in characteristics:
            if (aid, iid) not in self.subscriptions:
                continue

            char = self._get_characteristic_instance(aid, iid)

            tmp.append({
                'aid': aid,
                'iid': iid,
                'value': char.get_value(),
            })

        # Bail out if this connection isnt subscribing to any of these characteristics
        if not tmp:
            return

        body = json.dumps({'characteristics': tmp})

        event = [
            'EVENT/1.0 200 OK',
            'Content-Type: application/hap+json',
            'Content-Length: {}'.format(len(body)),
            '',
            body
        ]

        self.write_encrypted_bytes('\r\n'.join(event).encode('utf-8'))

    def write_encrypted_bytes(self, data):
        with self.write_lock:
            if AccessoryRequestHandler.DEBUG_CRYPT:
                self.log_message('response >%s<', data)
                self.log_message('len(response) %s', len(data))

            block_size = 1024
            out_data = bytearray()
            while len(data) > 0:
                block = data[:block_size]
                if AccessoryRequestHandler.DEBUG_CRYPT:
                    self.log_message('==> BLOCK: len %s', len(block))
                data = data[block_size:]

                len_bytes = len(block).to_bytes(2, byteorder='little')
                a2c_key = self.server.sessions[self.session_id]['accessory_to_controller_key']
                cnt_bytes = self.server.sessions[self.session_id]['accessory_to_controller_count'].\
                    to_bytes(8, byteorder='little')
                ciper_and_mac = chacha20_aead_encrypt(len_bytes, a2c_key, cnt_bytes, bytes([0, 0, 0, 0]), block)
                self.server.sessions[self.session_id]['accessory_to_controller_count'] += 1
                out_data += len_bytes + ciper_and_mac[0] + ciper_and_mac[1]

            try:
                self.orig_wfile.write(out_data)
                self.orig_wfile.flush()
            except ValueError:
                raise DisconnectedControllerError()

    def handle_one_request(self):
        """
        This is used to determine whether the request is encrypted or not. This is done by looking at the first bytes of
        the request. To be valid unencrypted HTTP call, it must be one of the methods defined in RFC7231 Section 4
        "Request Methods".
        """
        try:
            # make connection non blocking so the select can work
            self.connection.setblocking(0)
            ready = select.select([self.connection], [], [], 1)

            # no data was to be received, so we count up to track how many seconds in total this happened
            if not ready[0]:
                self.timeout_counter += 1

                # if this is above our configured timeout the connection gets closed
                if self.timeout_counter >= self.timeout:
                    self.close_connection = True
                return

            raw_peeked_data = self.rfile.peek(10)
            if len(raw_peeked_data) == 0:
                # since select says ready but no data is there, close the connection to prevent hidden busy waiting to
                # rise load
                self.close_connection = True
                return

            # data was received so reset the timeout handler
            self.timeout_counter = 0

            # RFC7230 Section 3 tells us, that US-ASCII is fine
            peeked_data = raw_peeked_data[:10]
            peeked_data = peeked_data.decode(encoding='ASCII')
            # self.log_message('deciding over: >%s<', peeked_data)
            # If the request line starts with a known HTTP verb, then use handle_one_request from super class
            if ' ' in peeked_data:
                method = peeked_data.split(' ', 1)[0]
                if method in self.VALID_METHODS:
                    self.server.sessions[self.session_id]['enrypted_connection'] = False
                    BaseHTTPRequestHandler.handle_one_request(self)
                    return
        except (socket.timeout, OSError) as e:
            # a read or a write timed out.  Discard this connection
            self.log_error(' %r', e)
            self.close_connection = True
            return
        except UnicodeDecodeError as e:
            # this just means it might be encrypted...
            self.log_debug('Unicode exception %s' % e)
            pass

        # the first 2 bytes are the length of the encrypted data to follow
        len_bytes = self.rfile.read(2)
        data_len = int.from_bytes(len_bytes, byteorder='little')

        # the auth tag is not counted, so add its length
        data = self.rfile.read(data_len + 16)
        if AccessoryRequestHandler.DEBUG_CRYPT:
            self.log_message('data >%i< >%s<', len(data), binascii.hexlify(data))

        # get the crypto key from the session
        c2a_key = self.server.sessions[self.session_id]['controller_to_accessory_key']

        # verify & decrypt the read data
        cnt_bytes = self.server.sessions[self.session_id]['controller_to_accessory_count'].to_bytes(8,
                                                                                                    byteorder='little')
        decrypted = chacha20_aead_decrypt(len_bytes, c2a_key, cnt_bytes, bytes([0, 0, 0, 0]),
                                          data)
        if decrypted is False:
            # crypto error, log it and request close of connection
            self.log_error('SEVERE: Could not decrypt %s', binascii.hexlify(data))
            self.close_connection = True
            return

        if AccessoryRequestHandler.DEBUG_CRYPT:
            self.log_message('crypted request >%s<', decrypted)

        self.server.sessions[self.session_id]['controller_to_accessory_count'] += 1

        # replace the original rfile with a fake with the decrypted stuff
        old_rfile = self.rfile
        self.rfile = io.BytesIO(decrypted)

        # replace writefile to pass on encrypted data
        old_wfile = self.wfile
        self.wfile = io.BytesIO()

        # call known function
        self.server.sessions[self.session_id]['enrypted_connection'] = True
        BaseHTTPRequestHandler.handle_one_request(self)

        # read the plaintext and send it out encrypted
        self.wfile.seek(0)
        in_data = self.wfile.read(65537)

        self.write_encrypted_bytes(in_data)

        # change back to originals to handle multiple calls
        self.rfile = old_rfile
        self.wfile = old_wfile

    def _get_characteristics(self):
        """
        As described on page 84
        :return:
        """
        if AccessoryRequestHandler.DEBUG_GET_CHARACTERISTICS:
            self.log_message('GET /characteristics')

        # analyse
        params = {}
        if '?' in self.path:
            params = {t.split('=')[0]: t.split('=')[1] for t in self.path.split('?')[1].split('&')}

        # handle id param
        ids = []
        if 'id' in params:
            ids = params['id'].split(',')

        # handle meta param
        meta = False
        if 'meta' in params:
            meta = params['meta'] == '1'

        # handle perms param
        perms = False
        if 'perms' in params:
            perms = params['perms'] == '1'

        # handle type param
        include_type = False
        if 'type' in params:
            include_type = params['type'] == '1'

        # handle ev param
        ev = False
        if 'ev' in params:
            ev = params['ev'] == '1'

        if AccessoryRequestHandler.DEBUG_GET_CHARACTERISTICS:
            self.log_message('query parameters: ids: %s, meta: %s, perms: %s, type: %s, ev: %s', ids, meta, perms,
                             include_type, ev)

        result = {
            'characteristics': []
        }

        errors = 0
        for id_pair in ids:
            id_pair = id_pair.split('.')
            aid = int(id_pair[0])
            cid = int(id_pair[1])
            found = False
            for accessory in self.server.accessories.accessories:
                if accessory.aid != aid:
                    continue
                for service in accessory.services:
                    for characteristic in service.characteristics:
                        if characteristic.iid != cid:
                            continue
                        found = True
                        # try to read the characteristic and report possible exceptions as error
                        try:
                            value = characteristic.get_value()
                            result['characteristics'].append({'aid': aid, 'iid': cid, 'value': value})
                        except FormatError:
                            result['characteristics'].append(
                                {'aid': aid, 'iid': cid, 'status': HapStatusCodes.INVALID_VALUE})
                            errors += 1
                        except CharacteristicPermissionError:
                            result['characteristics'].append(
                                {'aid': aid, 'iid': cid, 'status': HapStatusCodes.CANT_READ_WRITE_ONLY})
                            errors += 1
                        except Exception as e:
                            self.log_error('Exception while getting value for %s.%s: %s', aid, cid, str(e))
                            result['characteristics'].append(
                                {'aid': aid, 'iid': cid, 'status': HapStatusCodes.OUT_OF_RESOURCES})
                            errors += 1
                        if ev:
                            # TODO handling of events is missing
                            result['characteristics'][-1]['ev'] = (aid, cid) in self.subscriptions
                        if include_type:
                            result['characteristics'][-1]['type'] = \
                                CharacteristicsTypes.get_short_uuid(characteristic.type)
                        if perms:
                            result['characteristics'][-1]['perms'] = characteristic.perms
                        if meta:
                            meta_data = characteristic.get_meta()
                            for key in meta_data:
                                result['characteristics'][-1][key] = meta_data[key]
                # report missing resources
                if not found:
                    result['characteristics'].append(
                        {'aid': aid, 'iid': cid, 'status': HapStatusCodes.RESOURCE_NOT_EXIST})
                    errors += 1

        if AccessoryRequestHandler.DEBUG_GET_CHARACTERISTICS:
            self.log_message('chars: %s', json.dumps(result))

        # set the proper status code depending on the count of characteristics and error
        if len(result['characteristics']) == errors:
            self.send_response(HttpStatusCodes.BAD_REQUEST)
        elif len(result['characteristics']) > errors > 0:
            self.send_response(HttpStatusCodes.MULTI_STATUS)
        else:
            self.send_response(HttpStatusCodes.OK)

        result_bytes = json.dumps(result).encode()
        self.send_header('Content-Type', 'application/hap+json')
        self.send_header('Content-Length', len(result_bytes))
        self.end_headers()
        self.wfile.write(result_bytes)

    def _get_characteristic_instance(self, aid, iid):
        for accessory in self.server.accessories.accessories:
            if accessory.aid != aid:
                continue
            for service in accessory.services:
                for characteristic in service.characteristics:
                    if characteristic.iid != iid:
                        continue
                    return characteristic

    def _put_characteristics(self):
        """
        Defined page 80 ff
        :return:
        """
        if AccessoryRequestHandler.DEBUG_PUT_CHARACTERISTICS:
            self.log_message('PUT /characteristics')
            self.log_message('body: %s', self.body)

        data = json.loads(self.body.decode())
        characteristics_to_set = data['characteristics']
        result = {
            'characteristics': []
        }
        changed = []
        errors = 0
        for characteristic_to_set in characteristics_to_set:
            aid = characteristic_to_set['aid']
            cid = characteristic_to_set['iid']
            found = False
            for accessory in self.server.accessories.accessories:
                if accessory.aid != aid:
                    continue
                for service in accessory.services:
                    for characteristic in service.characteristics:
                        if characteristic.iid != cid:
                            continue
                        found = True
                        if 'ev' in characteristic_to_set:
                            if AccessoryRequestHandler.DEBUG_PUT_CHARACTERISTICS:
                                self.log_message('set ev >%s< >%s< >%s<', aid, cid, characteristic_to_set['ev'])
                            if 'ev' in characteristic.perms:
                                if characteristic_to_set['ev']:
                                    self.subscriptions.add((aid, cid))
                                else:
                                    self.subscriptions.discard((aid, cid))
                                result['characteristics'].append({'aid': aid, 'iid': cid, 'status': 0})
                            else:
                                result['characteristics'].append(
                                    {'aid': aid, 'iid': cid, 'status': HapStatusCodes.NOTIFICATION_NOT_SUPPORTED})

                        if 'value' in characteristic_to_set:
                            if AccessoryRequestHandler.DEBUG_PUT_CHARACTERISTICS:
                                self.log_message('set value >%s< >%s< >%s<', aid, cid, characteristic_to_set['value'])
                            try:
                                characteristic.set_value(characteristic_to_set['value'])
                                result['characteristics'].append({'aid': aid, 'iid': cid, 'status': 0})
                                changed.append((aid, cid))
                            except FormatError:
                                result['characteristics'].append(
                                    {'aid': aid, 'iid': cid, 'status': HapStatusCodes.INVALID_VALUE})
                                errors += 1
                            except Exception as e:
                                self.log_error('Exception while setting value for %s.%s: %s', aid, cid, str(e))
                                result['characteristics'].append(
                                    {'aid': aid, 'iid': cid, 'status': HapStatusCodes.OUT_OF_RESOURCES})
                                errors += 1
            # report missing resources
            if not found:
                result['characteristics'].append(
                    {'aid': aid, 'iid': cid, 'status': HapStatusCodes.RESOURCE_NOT_EXIST})
                errors += 1

        if changed:
            self.server.write_event(changed, self.session_id)

        if len(result['characteristics']) == errors:
            self.send_response(HttpStatusCodes.BAD_REQUEST)
        elif len(result['characteristics']) > errors > 0:
            self.send_response(HttpStatusCodes.MULTI_STATUS)
        else:
            self.send_response(HttpStatusCodes.NO_CONTENT)
            self.end_headers()
        if errors > 0:
            result_bytes = json.dumps(result).encode()
            self.send_header('Content-Type', 'application/hap+json')
            self.send_header('Content-Length', len(result_bytes))
            self.end_headers()
            self.wfile.write(result_bytes)

    def _post_identify(self):
        if self.server.data.is_paired:
            result_bytes = json.dumps({'status': HapStatusCodes.INSUFFICIENT_PRIVILEGES}).encode()
            self.send_response(HttpStatusCodes.BAD_REQUEST)
            self.send_header('Content-Type', 'application/hap+json')
            self.send_header('Content-Length', len(result_bytes))
            self.end_headers()
            self.wfile.write(result_bytes)
        else:
            # perform identify action
            if self.identify_callback:
                self.identify_callback()
            # send status code
            self.send_response(HttpStatusCodes.NO_CONTENT)
            self.end_headers()

    def _get_accessories(self):

        result_bytes = self.server.accessories.to_accessory_and_service_list().encode()
        self.send_response(HttpStatusCodes.OK)
        self.send_header('Content-Type', 'application/hap+json')
        self.send_header('Content-Length', len(result_bytes))
        self.end_headers()
        self.wfile.write(result_bytes)

    def _post_pair_verify(self):
        d_req = TLV.decode_bytes(self.body)

        # Order is not consistent, so force things in to order specified in spec
        d_req = TLV.reorder(d_req, [
            TLV.kTLVType_State,
            TLV.kTLVType_PublicKey,
            TLV.kTLVType_EncryptedData,
        ])

        d_res = []

        if d_req[0][0] == TLV.kTLVType_State and d_req[0][1] == TLV.M1:
            # step #2 Accessory -> iOS Device Verify Start Response
            if AccessoryRequestHandler.DEBUG_PAIR_VERIFY:
                self.log_message('Step #2 /pair-verify')

            # 1) generate new curve25519 key pair
            accessory_session_key = x25519.X25519PrivateKey.generate()
            accessory_spk = accessory_session_key.public_key().public_bytes(
                encoding=serialization.Encoding.Raw,
                format=serialization.PublicFormat.Raw
            )
            self.server.sessions[self.session_id]['accessory_pub_key'] = accessory_spk

            # 2) generate shared secret
            ios_device_curve25519_pub_key_bytes = bytes(d_req[1][1])
            self.server.sessions[self.session_id]['ios_device_pub_key'] = ios_device_curve25519_pub_key_bytes
            ios_device_curve25519_pub_key = x25519.X25519PublicKey.from_public_bytes(
                ios_device_curve25519_pub_key_bytes)

            shared_secret = accessory_session_key.exchange(ios_device_curve25519_pub_key)
            self.server.sessions[self.session_id]['shared_secret'] = shared_secret

            # 3) generate accessory info
            accessory_info = accessory_spk + self.server.data.accessory_pairing_id_bytes + \
                ios_device_curve25519_pub_key_bytes

            # 4) sign accessory info for accessory signature
            # accessory_ltsk = ed25519.SigningKey(self.server.data.accessory_ltsk + self.server.data.accessory_ltpk)
            accessory_ltsk = ed25519.SigningKey(self.server.data.accessory_ltsk)
            accessory_signature = accessory_ltsk.sign(accessory_info)

            # 5) sub tlv
            sub_tlv = [
                (TLV.kTLVType_Identifier, self.server.data.accessory_pairing_id_bytes),
                (TLV.kTLVType_Signature, accessory_signature)
            ]
            sub_tlv_b = TLV.encode_list(sub_tlv)

            # 6) derive session key
            hkdf_inst = hkdf.Hkdf('Pair-Verify-Encrypt-Salt'.encode(), shared_secret, hash=hashlib.sha512)
            session_key = hkdf_inst.expand('Pair-Verify-Encrypt-Info'.encode(), 32)
            self.server.sessions[self.session_id]['session_key'] = session_key

            # 7) encrypt sub tlv
            encrypted_data_with_auth_tag = chacha20_aead_encrypt(bytes(),
                                                                 session_key,
                                                                 'PV-Msg02'.encode(),
                                                                 bytes([0, 0, 0, 0]),
                                                                 sub_tlv_b)
            tmp = bytearray(encrypted_data_with_auth_tag[0])
            tmp += encrypted_data_with_auth_tag[1]

            # 8) construct result tlv
            d_res.append((TLV.kTLVType_State, TLV.M2,))
            d_res.append((TLV.kTLVType_PublicKey, accessory_spk))
            d_res.append((TLV.kTLVType_EncryptedData, tmp))

            self._send_response_tlv(d_res)
            if AccessoryRequestHandler.DEBUG_PAIR_VERIFY:
                self.log_message('after step #2\n%s', TLV.to_string(d_res))
            return

        if d_req[0][0] == TLV.kTLVType_State and d_req[0][1] == TLV.M3:
            # step #4 Accessory -> iOS Device Verify Finish Response
            if AccessoryRequestHandler.DEBUG_PAIR_VERIFY:
                self.log_message('Step #4 /pair-verify')

            session_key = self.server.sessions[self.session_id]['session_key']

            # 1) verify ios' authtag
            # 2) decrypt
            encrypted = d_req[1][1]
            decrypted = chacha20_aead_decrypt(bytes(), session_key, 'PV-Msg03'.encode(), bytes([0, 0, 0, 0]),
                                              encrypted)
            if decrypted is False:
                self.send_error_reply(TLV.M4, TLV.kTLVError_Authentication)
                self.log_error('error in step #4: authtag %s %s', d_res, self.server.sessions)
                return
            d1 = TLV.decode_bytes(decrypted)
            assert d1[0][0] == TLV.kTLVType_Identifier
            assert d1[1][0] == TLV.kTLVType_Signature

            # 3) get ios_device_ltpk
            ios_device_pairing_id = d1[0][1]
            self.server.sessions[self.session_id]['ios_device_pairing_id'] = ios_device_pairing_id
            ios_device_ltpk_bytes = self.server.data.get_peer_key(ios_device_pairing_id)
            if ios_device_ltpk_bytes is None:
                self.send_error_reply(TLV.M4, TLV.kTLVError_Authentication)
                self.log_error('error in step #4: not paired %s %s', d_res, self.server.sessions)
                return
            ios_device_lptk = ed25519.VerifyingKey(ios_device_ltpk_bytes)

            # 4) verify ios_device_info
            ios_device_sig = d1[1][1]
            ios_device_curve25519_pub_key_bytes = self.server.sessions[self.session_id]['ios_device_pub_key']
            accessory_spk = self.server.sessions[self.session_id]['accessory_pub_key']
            ios_device_info = ios_device_curve25519_pub_key_bytes + ios_device_pairing_id + accessory_spk
            try:
                ios_device_lptk.verify(bytes(ios_device_sig), bytes(ios_device_info))
            except ed25519.BadSignatureError:
                self.send_error_reply(TLV.M4, TLV.kTLVError_Authentication)
                self.log_error('error in step #4: signature %s %s', d_res, self.server.sessions)
                return

            #
            shared_secret = self.server.sessions[self.session_id]['shared_secret']
            hkdf_inst = hkdf.Hkdf('Control-Salt'.encode(), shared_secret, hash=hashlib.sha512)
            controller_to_accessory_key = hkdf_inst.expand('Control-Write-Encryption-Key'.encode(), 32)
            self.server.sessions[self.session_id]['controller_to_accessory_key'] = controller_to_accessory_key
            self.server.sessions[self.session_id]['controller_to_accessory_count'] = 0

            hkdf_inst = hkdf.Hkdf('Control-Salt'.encode(), shared_secret, hash=hashlib.sha512)
            accessory_to_controller_key = hkdf_inst.expand('Control-Read-Encryption-Key'.encode(), 32)
            self.server.sessions[self.session_id]['accessory_to_controller_key'] = accessory_to_controller_key
            self.server.sessions[self.session_id]['accessory_to_controller_count'] = 0

            d_res.append((TLV.kTLVType_State, TLV.M4,))

            self._send_response_tlv(d_res)
            if AccessoryRequestHandler.DEBUG_PAIR_VERIFY:
                self.log_message('after step #4\n%s', TLV.to_string(d_res))
            return

        self.send_error(HttpStatusCodes.METHOD_NOT_ALLOWED)

    def _post_pairings(self):
        d_req = TLV.decode_bytes(self.body)

        # Order is not consistent, so force things in to order specified in spec
        d_req = TLV.reorder(d_req, [
            TLV.kTLVType_State,
            TLV.kTLVType_Method,

            # AddPairing/RemovePairing, M1
            TLV.kTLVType_Identifier,

            # AddPairing, M1
            TLV.kTLVType_PublicKey,
            TLV.kTLVType_Permissions,
        ])

        self.log_message('POST /pairings request body:\n%s', TLV.to_string(d_req))

        session = self.server.sessions[self.session_id]
        server_data = self.server.data

        d_res = []

        if d_req[0][0] == TLV.kTLVType_State and d_req[0][1] == TLV.M1 \
                and d_req[1][0] == TLV.kTLVType_Method and d_req[1][1] == TLV.AddPairing:
            self.log_message('Step #2 /pairings add pairing')
            d_res.append((TLV.kTLVType_State, TLV.M2,))

            # see page 51
            # 1)

            # 2) verify admin bit is set
            ios_device_pairing_id = session['ios_device_pairing_id']
            if not server_data.is_peer_admin(ios_device_pairing_id):
                self.send_error_reply(TLV.M2, TLV.kTLVError_Authentication)
                self.log_error('error in step #2: admin bit')
                return

            additional_controller_pairing_identifier = d_req[2][1]
            additional_controller_LTPK = d_req[3][1]
            additional_controller_permissions = d_req[4][1]
            is_admin = additional_controller_permissions == b'\x01'

            # 3) pairing exists?
            registered_controller_LTPK = server_data.get_peer_key(additional_controller_pairing_identifier)

            if registered_controller_LTPK is not None:
                self.log_message('controller is already registered!')
                if registered_controller_LTPK != additional_controller_LTPK:
                    self.log_message('with different key')
                    # 3.a)
                    self.send_error_reply(TLV.M2, TLV.kTLVError_Unknown)
                    return

                self.log_message('with different permissions')
                # 3.b) update permission
                server_data.set_peer_permissions(additional_controller_pairing_identifier, is_admin)
            else:
                self.log_message('add pairing')

                # 4) no pairing exists
                # 4.a) no limit applied to number of pairings
                # 4.b) add pairing (could raise kTLVError_Unknown)
                server_data.add_peer(additional_controller_pairing_identifier, additional_controller_LTPK, is_admin)

            self._send_response_tlv(d_res)
            self.log_message('after step #2\n%s', TLV.to_string(d_res))

            return

        if d_req[0][0] == TLV.kTLVType_State and d_req[0][1] == TLV.M1 \
                and d_req[1][0] == TLV.kTLVType_Method and d_req[1][1] == TLV.RemovePairing:
            # step #2 Accessory -> iOS Device remove pairing response
            self.log_message('Step #2 /pairings remove pairings')

            # 1)

            # 2) verify set admin bit
            ios_device_pairing_id = session['ios_device_pairing_id']
            if not server_data.is_peer_admin(ios_device_pairing_id):
                self.send_error_reply(TLV.M2, TLV.kTLVError_Authentication)
                self.log_error('error in step #2: admin bit not set for controller %s', ios_device_pairing_id.decode())
                return

            # 3) remove pairing and republish device
            server_data.remove_peer(d_req[2][1])
            self.server.publish_device()

            d_res.append((TLV.kTLVType_State, TLV.M2,))
            self._send_response_tlv(d_res)
            self.log_message('after step #2\n%s', TLV.to_string(d_res))

            # 6) + 7) invalidate HAP session and close connections
            # TODO implement this in more details
            # for session_id in self.server.sessions:
            #    session = self.server.sessions[session_id]
            #    if session['ios_device_pairing_id'] == d_req[TLV.kTLVType_Identifier]:
            #        session['handler'].close_connection = True
            #
            # if self.server.sessions[self.session_id]['ios_device_pairing_id'] == d_req[TLV.kTLVType_Identifier]:
            #    self.close_connection = True
            return

        if d_req[0][0] == TLV.kTLVType_State and d_req[0][1] == TLV.M1 \
                and d_req[1][0] == TLV.kTLVType_Method and d_req[1][1] == TLV.ListPairings:
            # step #2 Accessory -> iOS Device list pairing response
            self.log_message('Step #2 /pairings list pairings')

            # 1) Validate against session

            # 2) verify set admin bit
            ios_device_pairing_id = session['ios_device_pairing_id']
            if not server_data.is_peer_admin(ios_device_pairing_id):
                self.send_error_reply(TLV.M2, TLV.kTLVError_Authentication)
                self.log_error('error in step #2: admin bit')
                return

            # 3) construct response TLV
            tmp = [(TLV.kTLVType_State, TLV.M2)]
            for index, pairing_id in enumerate(server_data.peers):
                tmp.append((TLV.kTLVType_Identifier, pairing_id.encode()))
                tmp.append((TLV.kTLVType_PublicKey, server_data.get_peer_key(pairing_id.encode())))
                user = TLV.kTLVType_Permission_RegularUser
                if server_data.is_peer_admin(pairing_id.encode()):
                    user = TLV.kTLVType_Permission_AdminUser
                tmp.append((TLV.kTLVType_Permissions, user))
                if index + 1 < len(server_data.peers):
                    tmp.append((TLV.kTLVType_Separator, bytes(0)))
            result_bytes = TLV.encode_list(tmp)

            # 4) send response
            self.send_response(HttpStatusCodes.OK)
            # Send headers
            self.send_header('Content-Length', len(result_bytes))
            self.send_header('Content-Type', 'application/pairing+tlv8')
            self.send_header('Connection', 'keep-alive')
            self.end_headers()

            self.wfile.write(result_bytes)
            return

        self.send_error(HttpStatusCodes.METHOD_NOT_ALLOWED)

    def send_error_reply(self, state, error):
        """
        Send an error reply encoded as TLV.
        :param state: The state as in TLV.M1, TLV.M2, ...
        :param error: The error code as in TLV.kTLVError_*
        :return: None
        """
        d_res = [
            (TLV.kTLVType_State, state),
            (TLV.kTLVType_Error, error)
        ]
        result_bytes = TLV.encode_list(d_res)

        self.send_response(HttpStatusCodes.METHOD_NOT_ALLOWED)
        # Send headers
        self.send_header('Content-Length', len(result_bytes))
        self.send_header('Content-Type', 'application/pairing+tlv8')
        self.end_headers()

        self.wfile.write(result_bytes)

    def _post_pair_setup(self):
        d_req = TLV.decode_bytes(self.body)

        # Order is not consistent, so force things in to order specified in spec
        d_req = TLV.reorder(d_req, [
            TLV.kTLVType_State,
            TLV.kTLVType_Method,
            # For M3
            TLV.kTLVType_PublicKey,
            TLV.kTLVType_Proof,
            # For M5
            TLV.kTLVType_EncryptedData,
        ])

        self.log_message('POST /pair-setup request body:\n%s', TLV.to_string(d_req))

        d_res = []

        if d_req[0][0] == TLV.kTLVType_State and d_req[0][1] == TLV.M1:
            # step #2 Accessory -> iOS Device SRP Start Response
            self.log_message('Step #2 /pair-setup')

            # 1) Check if paired
            if self.server.data.is_paired:
                self.send_error_reply(TLV.M2, TLV.kTLVError_Unavailable)
                return

            # 2) Check if over 100 attempts
            if self.server.data.unsuccessful_tries > 100:
                self.log_error('to many failed attempts')
                self.send_error_reply(TLV.M2, TLV.kTLVError_MaxTries)
                return

            # 3) Check if already in pairing
            if False:
                self.send_error_reply(TLV.M2, TLV.kTLVError_Busy)
                return

            # 4) 5) 7) Create in SRP Session, set username and password
            server = SrpServer('Pair-Setup', self.server.data.setup_code)

            # 6) create salt
            salt = server.get_salt()

            # 8) show setup code to user
            sc = self.server.data.setup_code
            sc_str = 'Setup Code\n┌─' + '─' * len(sc) + '─┐\n│ ' + sc + ' │\n└─' + '─' * len(sc) + '─┘'
            self.log_message(sc_str)

            # 9) create public key
            public_key = server.get_public_key()

            # 10) create response tlv and send response
            d_res.append((TLV.kTLVType_State, TLV.M2,))
            d_res.append((TLV.kTLVType_PublicKey, SrpServer.to_byte_array(public_key),))
            d_res.append((TLV.kTLVType_Salt, SrpServer.to_byte_array(salt),))
            self._send_response_tlv(d_res)

            # store session
            self.server.sessions[self.session_id]['srp'] = server
            self.log_message('after step #2:\n%s', TLV.to_string(d_res))
            return

        if d_req[0][0] == TLV.kTLVType_State and d_req[0][1] == TLV.M3:
            # step #4 Accessory -> iOS Device SRP Verify Response
            self.log_message('Step #4 /pair-setup')

            # 1) use ios pub key to compute shared secret key
            ios_pub_key = int.from_bytes(d_req[1][1], "big")
            server = self.server.sessions[self.session_id]['srp']
            server.set_client_public_key(ios_pub_key)

            hkdf_inst = hkdf.Hkdf('Pair-Setup-Encrypt-Salt'.encode(), SrpServer.to_byte_array(server.get_session_key()),
                                  hash=hashlib.sha512)
            session_key = hkdf_inst.expand('Pair-Setup-Encrypt-Info'.encode(), 32)
            self.server.sessions[self.session_id]['session_key'] = session_key

            # 2) verify ios proof
            ios_proof = int.from_bytes(d_req[2][1], "big")
            if not server.verify_clients_proof(ios_proof):
                d_res.append((TLV.kTLVType_State, TLV.M4,))
                d_res.append((TLV.kTLVType_Error, TLV.kTLVError_Authentication,))

                self._send_response_tlv(d_res)
                self.log_error('error in step #4 %s %s', d_res, self.server.sessions)
                return
            else:
                self.log_message('ios proof was verified')

            # 3) generate accessory proof
            accessory_proof = server.get_proof(ios_proof)

            # 4) create response tlv
            d_res.append((TLV.kTLVType_State, TLV.M4,))
            d_res.append((TLV.kTLVType_Proof, SrpServer.to_byte_array(accessory_proof),))

            # 5) send response tlv
            self._send_response_tlv(d_res)

            self.log_message('after step #4:\n%s', TLV.to_string(d_res))
            return

        if d_req[0][0] == TLV.kTLVType_State and d_req[0][1] == TLV.M5:
            # step #6 Accessory -> iOS Device Exchange Response
            self.log_message('Step #6 /pair-setup')

            # 1) Verify the iOS device's authTag
            # done by chacha20_aead_decrypt

            # 2) decrypt and test
            encrypted_data = d_req[1][1]
            decrypted_data = chacha20_aead_decrypt(bytes(), self.server.sessions[self.session_id]['session_key'],
                                                   'PS-Msg05'.encode(), bytes([0, 0, 0, 0]),
                                                   encrypted_data)
            if decrypted_data is False:
                d_res.append((TLV.kTLVType_State, TLV.M6,))
                d_res.append((TLV.kTLVType_Error, TLV.kTLVError_Authentication,))

                self.send_error_reply(TLV.M6, TLV.kTLVError_Authentication)
                self.log_error('error in step #6 %s %s', d_res, self.server.sessions)
                return

            d_req_2 = TLV.decode_bytearray(decrypted_data)

            # 3) Derive ios_device_x
            shared_secret = self.server.sessions[self.session_id]['srp'].get_session_key()
            hkdf_inst = hkdf.Hkdf('Pair-Setup-Controller-Sign-Salt'.encode(), SrpServer.to_byte_array(shared_secret),
                                  hash=hashlib.sha512)
            ios_device_x = hkdf_inst.expand('Pair-Setup-Controller-Sign-Info'.encode(), 32)

            # 4) construct ios_device_info
            ios_device_pairing_id = d_req_2[0][1]  # should be TLV.kTLVType_Identifier
            ios_device_ltpk = d_req_2[1][1]  # should be TLV.kTLVType_PublicKey
            ios_device_info = ios_device_x + ios_device_pairing_id + ios_device_ltpk

            # 5) verify signature
            ios_device_sig = d_req_2[2][1]  # should be [TLV.kTLVType_Signature

            verify_key = ed25519.VerifyingKey(bytes(ios_device_ltpk))
            try:
                verify_key.verify(bytes(ios_device_sig), bytes(ios_device_info))
            except ed25519.BadSignatureError:
                self.send_error_reply(TLV.M6, TLV.kTLVError_Authentication)
                self.log_error('error in step #6 %s %s', d_res, self.server.sessions)
                return

            # 6) save ios_device_pairing_id and ios_device_ltpk
            self.server.data.add_peer(ios_device_pairing_id, ios_device_ltpk, True)

            # Response Generation
            # 1) generate accessoryLTPK if not existing
            if self.server.data.accessory_ltsk is None or self.server.data.accessory_ltpk is None:
                accessory_ltsk, accessory_ltpk = ed25519.create_keypair()
                self.server.data.set_accessory_keys(
                    accessory_ltpk.to_bytes(),
                    accessory_ltsk.to_bytes(),
                )
            else:
                accessory_ltsk = ed25519.SigningKey(
                    self.server.data.accessory_ltsk + self.server.data.accessory_ltpk
                )
                accessory_ltpk = ed25519.VerifyingKey(self.server.data.accessory_ltpk)

            # 2) derive AccessoryX
            hkdf_inst = hkdf.Hkdf('Pair-Setup-Accessory-Sign-Salt'.encode(), SrpServer.to_byte_array(shared_secret),
                                  hash=hashlib.sha512)
            accessory_x = hkdf_inst.expand('Pair-Setup-Accessory-Sign-Info'.encode(), 32)

            # 3)
            accessory_info = accessory_x + self.server.data.accessory_pairing_id_bytes + accessory_ltpk.to_bytes()

            # 4) generate signature
            accessory_signature = accessory_ltsk.sign(accessory_info)

            # 5) construct sub_tlv
            sub_tlv = [
                (TLV.kTLVType_Identifier, self.server.data.accessory_pairing_id_bytes),
                (TLV.kTLVType_PublicKey, accessory_ltpk.to_bytes()),
                (TLV.kTLVType_Signature, accessory_signature)
            ]
            sub_tlv_b = TLV.encode_list(sub_tlv)

            # 6) encrypt sub_tlv
            encrypted_data_with_auth_tag = chacha20_aead_encrypt(bytes(),
                                                                 self.server.sessions[self.session_id]['session_key'],
                                                                 'PS-Msg06'.encode(),
                                                                 bytes([0, 0, 0, 0]),
                                                                 sub_tlv_b)
            tmp = bytearray(encrypted_data_with_auth_tag[0])
            tmp += encrypted_data_with_auth_tag[1]

            # 7) send response
            self.server.publish_device()
            d_res = [
                (TLV.kTLVType_State, TLV.M6,),
                (TLV.kTLVType_EncryptedData, tmp,)
            ]

            self._send_response_tlv(d_res)
            self.log_message('after step #6:\n%s', TLV.to_string(d_res))
            return

        self.send_error(HttpStatusCodes.METHOD_NOT_ALLOWED)

    def _send_response_tlv(self, d_res, close=False, status=HttpStatusCodes.OK):
        result_bytes = TLV.encode_list(d_res)

        self.send_response(status)
        # Send headers
        self.send_header('Content-Length', len(result_bytes))
        self.send_header('Content-Type', 'application/pairing+tlv8')
        self.send_header('Connection', 'keep-alive')
        self.end_headers()

        self.wfile.write(result_bytes)

    class Wrapper:
        """
        Wraps a bytes or byte array data into a file like object.
        """

        def __init__(self, data):
            self.data = data

        def makefile(self, arg):
            return io.BytesIO(self.data)

    def do_GET(self):
        """
        Can use
            * command
            * headers
            * path
            * ...
        :return:
        """
        absolute_path = self.path.split('?')[0]
        if absolute_path in self.PATHMAPPING:
            if 'GET' in self.PATHMAPPING[absolute_path]:
                # self.log_message('-' * 80 + '\ndo_GET / path: %s', self.path)
                self.PATHMAPPING[absolute_path]['GET']()
                return
        self.log_error('send error because of unmapped path: %s', self.path)
        self.send_error(HttpStatusCodes.NOT_FOUND)

    def do_POST(self):
        # read the body identified by its length
        content_length = int(self.headers['Content-Length'])
        self.body = self.rfile.read(content_length)
        if self.path in self.PATHMAPPING:
            if 'POST' in self.PATHMAPPING[self.path]:
                # self.log_message('-' * 80 + '\ndo_POST / path: %s', self.path)
                self.PATHMAPPING[self.path]['POST']()
                return
        self.log_error('send error because of unmapped path: %s', self.path)
        self.send_error(HttpStatusCodes.NOT_FOUND)

    def do_PUT(self):
        # read the body identified by its length
        content_length = int(self.headers['Content-Length'])
        self.body = self.rfile.read(content_length)
        if self.path in self.PATHMAPPING:
            if 'PUT' in self.PATHMAPPING[self.path]:
                # self.log_message('-' * 80 + '\ndo_PUT / path: %s', self.path)
                self.PATHMAPPING[self.path]['PUT']()
                return
        self.log_error('send error because of unmapped path: %s', self.path)
        self.send_error(HttpStatusCodes.NOT_FOUND)

    def log_message(self, format, *args):
        if self.server.logger is None:
            pass
        elif self.server.logger == sys.stderr:
            BaseHTTPRequestHandler.log_message(self, format, *args)
        else:
            self.server.logger.info("%s" % (format % args))

    def log_debug(self, format, *args):
        if self.server.logger is None:
            pass
        elif self.server.logger == sys.stderr:
            BaseHTTPRequestHandler.log_message(self, format, *args)
        else:
            self.server.logger.debug("%s" % (format % args))

    def log_error(self, format, *args):
        if self.server.logger is None:
            pass
        elif self.server.logger == sys.stderr:
            BaseHTTPRequestHandler.log_error(self, format, *args)
        else:
            self.server.logger.error("%s" % (format % args))
