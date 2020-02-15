
import sys
import json
from homekit.model.characteristics import CharacteristicsTypes
from homekit.model.services import ServicesTypes
from termcolor import colored, cprint

import argparse, sys

parser = argparse.ArgumentParser()
parser.add_argument('filename', nargs='?')
args = parser.parse_args()

if args.filename:    
  with open('accessory.json', 'r') as f:
    data = json.load(f)
elif not sys.stdin.isatty():
    inp = sys.stdin.read()
    data = json.load(inp)
else:
    parser.print_help()


for accessory in data["accessories"]:
    aid = accessory['aid']
    for service in accessory['services']:
        s_type = service['type']
        s_iid = service['iid']
        print('{aid}.{iid}: >{stype}<'.format(aid=aid, iid=s_iid, stype=ServicesTypes.get_short(s_type)))

        for characteristic in service['characteristics']:
            c_iid = characteristic['iid']
            value = characteristic.get('value', '')


            c_type = characteristic['type']
            perms = ','.join(characteristic['perms'])
            desc = characteristic.get('description', '')
            c_type = CharacteristicsTypes.get_short(c_type)
            if 'maxLen' in characteristic:
                maxLen = characteristic.get('maxLen')
                            
                if value is not None and len(value) <= maxLen:
                    cprint('  {aid}.{iid}: {value} [len: {length} <= {maxLen}] ({description}) >{ctype}< [{perms}]'.format(aid=aid,
                                                                                      iid=c_iid,
                                                                                      value=value,
                                                                                      length=len(value),
                                                                                      maxLen=maxLen,
                                                                                      ctype=c_type,
                                                                                      perms=perms,
                                                                                      description=desc), "green")
                elif value is None:

                    print('  {aid}.{iid}: {value} [{maxLen}] ({description}) >{ctype}< [{perms}]'.format(aid=aid,
                                                                                      iid=c_iid,
                                                                                      value=value,
                                                                                      maxLen=maxLen,
                                                                                      ctype=c_type,
                                                                                      perms=perms,
                                                                                      description=desc))
                else:
                    cprint('  {aid}.{iid}: {value} [len: {length} <= {maxLen}] ({description}) >{ctype}< [{perms}]'.format(aid=aid,
                                                                                      iid=c_iid,
                                                                                      value=value,
                                                                                      length=len(value),
                                                                                      maxLen=maxLen,
                                                                                      ctype=c_type,
                                                                                      perms=perms,
                                                                                      description=desc), "red")
                
            elif 'minValue' in characteristic and 'maxValue' in characteristic:

                minValue = characteristic.get('minValue')
                maxValue = characteristic.get('maxValue')

                if minValue <= value and value <= maxValue:
                    # cprint('  {minValue} < {value} < {maxValue}: '.format(minValue=minValue, value=value, maxValue=maxValue ), "green")
                    cprint('  {aid}.{iid}: {minValue}:> {value} <:{maxValue} ({description}) >{ctype}< [{perms}]'.format(aid=aid,
                                                                                      iid=c_iid,
                                                                                      value=value,
                                                                                      ctype=c_type,
                                                                                      perms=perms,
                                                                                      description=desc,
                                                                                      minValue=minValue,
                                                                                      maxValue=maxValue), "green")
                else:
                    cprint('  {aid}.{iid}: {minValue}>: {value} <:{maxValue} ({description}) >{ctype}< [{perms}]'.format(aid=aid,
                                                                                      iid=c_iid,
                                                                                      value=value,
                                                                                      ctype=c_type,
                                                                                      perms=perms,
                                                                                      description=desc,
                                                                                      minValue=minValue,
                                                                                      maxValue=maxValue), "red")
            else:
                print('  {aid}.{iid}: {value} ({description}) >{ctype}< [{perms}]'.format(aid=aid,
                                                                                      iid=c_iid,
                                                                                      value=value,
                                                                                      ctype=c_type,
                                                                                      perms=perms,
                                                                                      description=desc))
