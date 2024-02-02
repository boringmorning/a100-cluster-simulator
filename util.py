import numpy as np

ALGO = ["bf", "sp", "wf"]

for k in range(0, 5):
    print("load" + str(k) + ":")
    T = 1e9
    utils = []
    for algo in ALGO:
        fname = "util/load" + str(k) + "_" + algo  + ".txt"
        f = open(fname, "r")
        last_util = 0.0
        last_time = 0
        util = []
        for x in f:
            x = x.split()
            if len(x) == 1:
                T = int(x[0])
            else:
                t = int(x[0]) 
                for i in range(last_time, t):
                    util.append(last_util)
                last_time = t
                last_util = float(x[1])
        utils.append(util)
        f.close()

    for i in range(len(ALGO)):
        util = utils[i][:T]
        print("\t", ALGO[i], ": ", sum(util) / len(util))