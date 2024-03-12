import numpy as np
import random
import sys

slice = [1, 2, 3, 4, 7]
dist = [0.2, 0.4, 0.6, 0.8]
njob = 1000
batchSize = 1

NCONFIG = int(sys.argv[1])
NCASE = int(sys.argv[2])

for k in range(0, NCONFIG):
    for l in range(0, NCASE):
        t = []
        fname = "workload/load" + str(k) + "_" + str(l) + ".txt"
        f = open(fname, "w")
        t = np.random.poisson(3.4 - 0.2 * k, njob)
        for i in range(1, len(t)):
            t[i] = t[i-1] + t[i]
        for i in range(0, len(t)):
            t[i] = t[i - i % batchSize + batchSize // 2]

        for i in range(njob):
            x = random.random()
            sliceIdx = len(slice)-1
            for j in range(len(slice)-1):
                if x < dist[j]:
                    sliceIdx = j
                    break
            seq = random.randint(10,100)
            para = random.randint(50,500)
            runtime = []
            for j in slice:
                runtime.append(seq + para // j)
            f.write(str(int(t[i])) + " " + ' '.join(str(rt) for rt in runtime) + " " + str(slice[sliceIdx]) + "\n")

        f.close()