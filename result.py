import numpy as np
import sys

ALGO = ["wf", "bf", "my"]

NCONFIG = int(sys.argv[1])
NCASE = int(sys.argv[2])
T = 0

for k in range(0, NCONFIG):
    print("load" + str(k) + ":")
    utils = [0.0] * len(ALGO)
    qts = [0.0] * len(ALGO)
    for n in range(0, NCASE):
        for idx, algo in enumerate(ALGO):
            fname = "util/load" + str(k) + "_" + str(n) + "_"  + algo  + ".txt"
            f = open(fname, "r")
            last_util = 0.0
            last_time = 0
            util = []
            for x in f:
                x = x.split()
                if len(x) == 1:
                    T = int(x[0])
                elif len(x) == 3:
                    qts[idx] += float(x[0])
                else:
                    t = int(x[0]) 
                    for i in range(last_time, t):
                        util.append(last_util)
                    last_time = t
                    last_util = float(x[1])
            # utils.append(util)
            util = util[:T]
            utils[idx] += sum(util) / len(util)
            f.close()

    for i in range(len(ALGO)):
        qt = round(qts[i] / NCASE, 1)
        util = round(utils[i] / NCASE * 100, 1)
        print("\t", ALGO[i], ": ", util, " " , qt)