# follow.py
#
# Follow a file like tail -f.

import time
import re

from datetime import datetime
import matplotlib.pyplot as plt
import numpy as np




# Use plt ion 
plt.ion()
fig = plt.figure()
ax1 = fig.add_subplot(211)


def follow(thefile):
    thefile.seek(0,2)
    while True:
        line = thefile.readline()
        if not line:
            time.sleep(0.1)
            continue
        yield line


if __name__ == '__main__':

    # draw the figure so the animations will work
    fig = plt.gcf()
    fig.show()
    fig.canvas.draw()
    

    logfile = open("./monitor.log","r")
    loglines = follow(logfile)
    for line in loglines:
        #print(line.replace("\n", ""))
        matchObj = re.search( r"(\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}.\d{3})\sHAPServer->heap\s\[\s*\]\scurrent:\s(\d*)\s-\sminimum:\s(\d*)\s?\[clients:(\d*)\]\s\[queue:(\d*)\]", line, re.M|re.I)
        if matchObj:
            #print(matchObj)
            #print("matchObj.group(1) : {}".format(matchObj.group(1))
            print(matchObj.group(1), matchObj.group(2), matchObj.group(3), matchObj.group(4), matchObj.group(5))
            #print()
            #plt.scatter(matchObj.group(1), matchObj.group(2))
            #plt.pause(0.05)
            # update canvas immediately
            
            # Plot.
            xs = matchObj.group(1)
            ys = int(matchObj.group(2))
            ax1.scatter(xs, ys) 
            # update the figure.
            plt.plot(xs, ys, color="red")
            # Draw the line.
            fig.canvas.draw()
            # Clear the current plot.
            ax1.clear()
    
        