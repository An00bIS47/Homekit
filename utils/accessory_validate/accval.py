#!/usr/bin/env python

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
    #inp = sys.stdin.read()
    data = json.load(sys.stdin)    
    data = {"accessories": data}        
else:
    parser.print_help()


assertions = []
success = True

for accessory in data["accessories"]:
    aid = accessory['aid']
    for service in accessory['services']:
        s_type = service['type']
        s_iid = service['iid']
        
        #print('{aid}.{iid}: >{stype}<'.format(aid=aid, iid=s_iid, stype=ServicesTypes.get_short(s_type)))

        for characteristic in service['characteristics']:
            c_iid = characteristic['iid']
            value = characteristic.get('value', '')

            assertion = {}

            c_type = characteristic['type']
            perms = ','.join(characteristic['perms'])
            desc = characteristic.get('description', '')
            c_type = CharacteristicsTypes.get_short(c_type)
            if 'maxLen' in characteristic:
                maxLen = characteristic.get('maxLen')
                            
                if value is not None and len(value) <= maxLen:
                    # cprint('  {aid}.{iid}: {value} [len: {length} <= {maxLen}] ({description}) >{ctype}< [{perms}]'.format(aid=aid,
                    #                                                                   iid=c_iid,
                    #                                                                   value=value,
                    #                                                                   length=len(value),
                    #                                                                   maxLen=maxLen,
                    #                                                                   ctype=c_type,
                    #                                                                   perms=perms,
                    #                                                                   description=desc), "green")


                    assertion["text"] = "Check maxLen of {}.{}: Length: {}/{}".format(aid, c_iid, len(value), maxLen)
                    assertion["result"] = True
                    assertions.append(assertion)
                elif value is None:

                    # print('  {aid}.{iid}: {value} [{maxLen}] ({description}) >{ctype}< [{perms}]'.format(aid=aid,
                    #                                                                   iid=c_iid,
                    #                                                                   value=value,
                    #                                                                   maxLen=maxLen,
                    #                                                                   ctype=c_type,
                    #                                                                   perms=perms,
                    #                                                                   description=desc))
                    pass
                else:
                    cprint('  {aid}.{iid}: {value} [len: {length} <= {maxLen}] ({description}) >{ctype}< [{perms}]'.format(aid=aid,
                                                                                      iid=c_iid,
                                                                                      value=value,
                                                                                      length=len(value),
                                                                                      maxLen=maxLen,
                                                                                      ctype=c_type,
                                                                                      perms=perms,
                                                                                      description=desc), "red")

                    assertion["text"] = "Check maxLen of {}.{}: Length: {}/{}".format(aid, c_iid, len(value), maxLen)
                    assertion["result"] = False
                    assertions.append(assertion)
                    success = False
                
            elif 'minValue' in characteristic and 'maxValue' in characteristic:

                minValue = characteristic.get('minValue')
                maxValue = characteristic.get('maxValue')

                if minValue <= value and value <= maxValue:      
                    # cprint('  {aid}.{iid}: {minValue}:> {value} <:{maxValue} ({description}) >{ctype}< [{perms}]'.format(aid=aid,
                    #                                                                   iid=c_iid,
                    #                                                                   value=value,
                    #                                                                   ctype=c_type,
                    #                                                                   perms=perms,
                    #                                                                   description=desc,
                    #                                                                   minValue=minValue,
                    #                                                                   maxValue=maxValue), "green")
                    
                    assertion["text"] = "Check value of {}.{}: {} < {} < {}".format(aid, c_iid, minValue, value, maxValue)
                    assertion["result"] = True
                    assertions.append(assertion)
                else:
                    cprint('  {aid}.{iid}: {minValue}>: {value} <:{maxValue} ({description}) >{ctype}< [{perms}]'.format(aid=aid,
                                                                                      iid=c_iid,
                                                                                      value=value,
                                                                                      ctype=c_type,
                                                                                      perms=perms,
                                                                                      description=desc,
                                                                                      minValue=minValue,
                                                                                      maxValue=maxValue), "red")
                    assertion["text"] = "Check value of {}.{}: {} < {} < {}".format(aid, c_iid, minValue, value, maxValue)
                    assertion.result = False
                    assertions.append(assertion)
                    success = False
            else:
                # print('  {aid}.{iid}: {value} ({description}) >{ctype}< [{perms}]'.format(aid=aid,
                #                                                                       iid=c_iid,
                #                                                                       value=value,
                #                                                                       ctype=c_type,
                #                                                                       perms=perms,
                #                                                                       description=desc))
                pass
#print(assertions)
#sys.exit(!success)
if success == True:
  sys.exit(0)
sys.exit(1)