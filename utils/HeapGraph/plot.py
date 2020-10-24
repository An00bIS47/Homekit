
import numpy  # Import numpy
import matplotlib.pyplot as plt #import matplotlib library
from drawnow import *
import time 
import re

NoOfRecords = 3600
currentHeapValues = []
minimumHeapValues = []
clientValues = []
queueValues  = []


plt.ion() #Tell matplotlib you want interactive mode to plot live data
cnt=0

def follow(thefile):
    thefile.seek(0,2)
    while True:
        line = thefile.readline()
        if not line:
            time.sleep(0.1)
            continue
        yield line

def makeFig(): #Create a function that makes our desired plot
    #plt.ylim(59000,61000)                                 #Set y min and max values
    plt.title('Homekit Heap Analysing')      #Plot the title
    plt.grid(True)                                  #Turn the grid on
    plt.ylabel('Heap')                            #Set ylabels
    plt.plot(currentHeapValues, 'r-', label='current')       #plot the temperature
    # plt.plot(currentHeapValues, 'ro-', label='current')       #plot the temperature
    # plt.plot(minimumHeapValues, 'b^-', label='minimum') #plot pressure data
    plt.legend(loc='lower left')                    #plot the legend
    
    plt2=plt.twinx()                                  #Create a second y axis
    plt.ylim(0,20)                           #Set limits of second y axis- adjust to readings you are getting
    plt2.plot(clientValues, label='clients') #plot pressure data
    plt2.plot(queueValues,  label='queue') #plot pressure data
    plt2.set_ylabel('Clients + Queue')                    #label second y axis
    plt2.ticklabel_format(useOffset=False)           #Force matplotlib to NOT autoscale y axis
    plt2.legend(loc='lower right')                  #plot the legend
    
 
while True: # While loop that loops forever
    logfile = open("../../monitor.log","r")
    loglines = follow(logfile)
    for line in loglines:
        print(line, end="")
        matchObj = re.search( r"(.*)\sHAPServer->heap\s\[+.{3}]\scurrent:\s(.*)\s-\sminimum:\s(.*)\s\[clients:(.*)\]\s\[queue:(.*)\]", line, re.M|re.I)
        if matchObj:            

            curHeap = int(matchObj.group(2))
            minHeap = int(matchObj.group(3))
            clientVal = int(matchObj.group(4))
            queueVal = int(matchObj.group(5))
            print(curHeap, minHeap, clientVal, queueVal)
            # temp = float( dataArray[0])            #Convert first element to floating number and put in temp
            # P =    float( dataArray[1])            #Convert second element to floating number and put in P
            currentHeapValues.append(curHeap)                        #Build our tempF array by appending temp readings
            minimumHeapValues.append(minHeap)                     #Building our pressure array by appending P readings

            clientValues.append(clientVal)
            queueValues.append(queueVal)

            drawnow(makeFig)                       #Call drawnow to update our live graph
            plt.pause(.000001)                     #Pause Briefly. Important to keep drawnow from crashing
            cnt=cnt+1

            if(cnt > NoOfRecords):                            #If you have 50 or more points, delete the first one from the array
                currentHeapValues.pop(0)                       #This allows us to just see the last 50 data points
                minimumHeapValues.pop(0)
                clientValues.pop(0)
                queueValues.pop(0)
