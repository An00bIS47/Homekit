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

"""
This module is for regression tests.

This is where we have identified something that breaks a HomeKit accessory
certified by Apple because our implementation of the spec is different to
Apple's. If your change trips a test in this module it is likely you will
break support for a device that currently works.

We strive to comply with the HAP spec wherever possible, and where
possible we aim to do what an iOS device would do.
"""

import unittest
from unittest import mock

from homekit.controller.ip_implementation import IpPairing
from homekit.http_impl import HomeKitHTTPConnection
from homekit.http_impl.secure_http import SecureHttp
from homekit.protocol import create_ip_pair_setup_write, create_ip_pair_verify_write


class TestHTTPPairing(unittest.TestCase):

    """
    Communication failures in the pairing stage.

    These types of problem generally involve comparing a working and
    non-working device via WireShark.
    """

    def test_pair_setup_doesnt_add_extra_headers(self):
        """
        The tado internet bridge will fail if a pairing request has
        extraneous headers like `Accept-Encoding`.

        https://github.com/home-assistant/home-assistant/issues/16971
        https://github.com/jlusiardi/homekit_python/pull/130
        """
        connection = HomeKitHTTPConnection('localhost')
        write_fun = create_ip_pair_setup_write(connection)

        with mock.patch.object(connection, 'response_class') as resp:
            resp.return_value.read.return_value = b'\x00\x01\x01\x06\x01\x01'
            with mock.patch.object(connection, 'send') as send:
                write_fun(b'', [])
                assert b'accept-encoding' not in send.call_args[0][0].lower()

    def test_pair_verify_doesnt_add_extra_headers(self):
        """
        The tado internet bridge will fail if a pairing request has
        extraneous headers like `Accept-Encoding`.

        https://github.com/home-assistant/home-assistant/issues/16971
        https://github.com/jlusiardi/homekit_python/pull/130
        """
        connection = HomeKitHTTPConnection('localhost')
        write_fun = create_ip_pair_verify_write(connection)

        with mock.patch.object(connection, 'response_class') as resp:
            resp.return_value.read.return_value = b'\x00\x01\x01\x06\x01\x01'
            with mock.patch.object(connection, 'send') as send:
                write_fun(b'', [])
                assert b'accept-encoding' not in send.call_args[0][0].lower()


class TestSecureSession(unittest.TestCase):

    """
    Communication failures of HTTP secure session layer.

    To debug these its often easiest to modify demoserver.py to dump
    data as it decrypts it, then compare an iOS device to homekit_python.
    """

    def test_requests_have_host_header(self):
        """
        The tado internet bridge will fail if a secure session request
        doesn't have a Host header.

        https://github.com/home-assistant/home-assistant/issues/16971
        https://github.com/jlusiardi/homekit_python/pull/130
        """

        session = mock.Mock()
        session.pairing_data = {
            'AccessoryIP': '192.168.1.2',
            'AccessoryPort': 8080,
        }
        secure_http = SecureHttp(session)

        with mock.patch.object(secure_http, '_handle_request') as handle_req:
            secure_http.get('/characteristics')
            print(handle_req.call_args[0][0])
            assert '\r\nHost: 192.168.1.2:8080\r\n' in handle_req.call_args[0][0].decode()

            secure_http.post('/characteristics', b'')
            assert '\r\nHost: 192.168.1.2:8080\r\n' in handle_req.call_args[0][0].decode()

            secure_http.put('/characteristics', b'')
            assert '\r\nHost: 192.168.1.2:8080\r\n' in handle_req.call_args[0][0].decode()

    def test_requests_only_send_params_for_true_case(self):
        """
        The tado internet bridge will fail if a GET request has what it
        considers to be unexpected request parameters.

        An iPhone client sends requests like `/characteristics?id=1.10`.
        It doesn't transmit `ev=0` or any other falsies.

        https://github.com/home-assistant/home-assistant/issues/16971
        https://github.com/jlusiardi/homekit_python/pull/132
        """
        pairing = IpPairing({})
        with mock.patch.object(pairing, 'session') as session:
            session.get.return_value.read.return_value = b'{"characteristics": []}'

            pairing.get_characteristics([(1, 2)])
            assert session.get.call_args[0][0] == '/characteristics?id=1.2'

            pairing.get_characteristics([(1, 2)], include_meta=True)
            assert session.get.call_args[0][0] == '/characteristics?id=1.2&meta=1'
