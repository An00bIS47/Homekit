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

import unittest

from homekit.model.feature_flags import FeatureFlags


class TestFeatureFlags(unittest.TestCase):

    def test_no_support_hap_pairing(self):
        self.assertEqual(FeatureFlags[0], 'No support for HAP Pairing')

    def test_support_hap_pairing(self):
        self.assertEqual(FeatureFlags[1], 'Supports HAP Pairing')

    def test_bug_143(self):
        # 0b10 -> 2 means no hap pairing support?
        self.assertEqual(FeatureFlags[2], 'No support for HAP Pairing')

#    def test_unknown_code(self):
#        self.assertRaises(KeyError, FeatureFlags.__getitem__, 99)
