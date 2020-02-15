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

from homekit.model import ToDictMixin


class AbstractService(ToDictMixin):
    def __init__(self, service_type: str, iid: int):
        if type(self) is AbstractService:
            raise Exception('AbstractService is an abstract class and cannot be instantiated directly')
        self.type = service_type
        self.iid = iid
        self.characteristics = []

    def append_characteristic(self, characteristic):
        """
        Append the given characteristic to the service.

        :param characteristic: a subclass of AbstractCharacteristic
        """
        self.characteristics.append(characteristic)

    def to_accessory_and_service_list(self):
        characteristics_list = []
        for c in self.characteristics:
            characteristics_list.append(c.to_accessory_and_service_list())
        d = {
            'iid': self.iid,
            'type': self.type,
            'characteristics': characteristics_list
        }
        return d
