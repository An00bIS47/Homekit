#!/usr/bin/env python
# encoding: utf-8

"""
git describe --abbrev=0 --tags --always
"""
import os
import configparser
import sys
import io
import subprocess

data = io.StringIO('\n'.join(line.strip() for line in open("../../.gitmodules", "r")))
config = configparser.ConfigParser()
config.readfp(data)
sections = config.sections()

with open("init-modules.sh", "w") as f :
    f.write("#!/bin/sh\n")
    print("| Name | Version | URL |")
    print("|-------------|-------------|----------------|")

    for s in sections :        
        path = config.get(s, "path")
        url = config.get(s, "url")
        f.write("git submodule add %s %s\n" % (url, path))


        submodule_path = s.split(" ")[1].replace("\"", "")
        name = submodule_path.split("/")[-1]
        

        #print(submodule_path, name)
        if not submodule_path.startswith("utils"):
            os.chdir("../../" + submodule_path)
            out = subprocess.Popen(['git', 'describe', '--abbrev=0', '--tags', '--always'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            stdout, stderr = out.communicate()
            
            version = stdout.decode("utf-8").replace("\n", "")
            print("| " + name + " | " + version + " | " + url + " | ")

    f.close()