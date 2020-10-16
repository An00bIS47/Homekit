
import glob
import os

# set(HAP_SRC
#   ... c & cpp
#)
cmakeFile = "./CMakeLists.txt"
projectDir = "/Users/michael/Development/Homekit"
projectSources = []
projectIncludes = []


def getFilesWithExt(mypath, ext):
    srcFiles = [mypath[1:] + os.path.basename(x) for x in glob.glob(projectDir + mypath + "*." + ext)]    
    return srcFiles


def getSourceFiles(mypath):
    if not mypath.endswith("/"):
        mypath = mypath + "/"
    cppFiles = getFilesWithExt(mypath, "cpp")   
    cFiles = getFilesWithExt(mypath, "c")
    return cppFiles + cFiles

def genCmakeSetSources(src, files):
    result = "\n"
    result += "set(" + src + "\n"

    for f in files:
        result += "\t" + f + "\n"
    result += ")\n"
    return result

def appendToFile(file, text):
    with open(file, "a") as myfile:
        myfile.write(text)

def getSubDirs(mypath):
    if not mypath.endswith("/"):
        mypath = mypath + "/"
    return [mypath + os.path.basename(x[0]) for x in os.walk(projectDir + mypath)]        

def getSourcesName(mypath, prefix=""):
    s = mypath.split("/")
    return prefix + s[-1].upper() + "_SRCS"


def appendSourceFolder(mypath, prefix=""):
    srcFiles = getSourceFiles(mypath)        
    sourceName = getSourcesName(mypath, prefix)
    appendToFile(cmakeFile, genCmakeSetSources(sourceName, srcFiles))
    projectSources.append(sourceName)
    projectIncludes.append(mypath)


# appendToFile(cmakeFile, "\ncmake_minimum_required(VERSION 3.5)\n")
# appendToFile(cmakeFile, "\ninclude($ENV{IDF_PATH}/tools/cmake/project.cmake)\n")
# appendToFile(cmakeFile, "\nproject(Homekit)\n")

appendSourceFolder("/src/HAP")
appendSourceFolder("/src/Crypto")

pluginDirs = getSubDirs("/src/HAP/plugins/")
for p in pluginDirs:
    # if p != "/src/HAP/plugins/":
    appendSourceFolder(p, "PLUGIN_")


appendToFile(cmakeFile, "\nset(COMPONENT_SRCS\n")
for s in projectSources:
    appendToFile(cmakeFile, "\t${")
    appendToFile(cmakeFile, s)
    appendToFile(cmakeFile, "}\n")
appendToFile(cmakeFile, ")\n")    


appendToFile(cmakeFile, "\nset(COMPONENT_ADD_INCLUDEDIRS\n")
for i in projectIncludes:
    appendToFile(cmakeFile, "\t")
    appendToFile(cmakeFile, i)
    appendToFile(cmakeFile, "\n")
appendToFile(cmakeFile, ")\n")  

appendToFile(cmakeFile, "\nregister_component()\n")
