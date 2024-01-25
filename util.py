import numpy as np

ALGO = ["_bf", "_my", "_wf"]

for k in range(0, 5):
    print("load" + str(k) + ":")
    for algo in ALGO:
        fname = "util/load" + str(k) + algo  +".txt"
        f = open(fname, "r")
        last_util = 0.0
        last_time = 0
        util = []
        for x in f:
            x = x[0:len(x)-1]
            x = x.split()
            t = int(x[0]) 
            for i in range(last_time, t):
                util.append(last_util)
            last_time = t
            last_util = float(x[1])
        util = util[:1600]
        print("\t", sum(util) / len(util))

        f.close()