import numpy as np

ALGO = ["_final", "_my"]

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
        prefix = []
        ps = 0
        w = 0
        for u in util:
            w += 1
            ps += u
            prefix.append(ps / w)
        print("\t", sum(prefix) / len(prefix))

        f.close()