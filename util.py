import numpy as np

ALGO = ["bf", "sp", "wf"]

for k in range(0, 3):
    print("load" + str(k) + ":")
    utils = [0.0, 0.0, 0.0]
    for n in range(0, 5):
        T = 1e9
        for idx, algo in enumerate(ALGO):
            fname = "util/load" + str(k*5 + n) + "_" + algo  + ".txt"
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
            # utils.append(util)
            util = util[:T]
            utils[idx] += sum(util) / len(util)
            f.close()

    for i in range(len(ALGO)):
        util = utils[i] / 5
        print("\t", ALGO[i], ": ", util)