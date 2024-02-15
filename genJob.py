import numpy as np
import random

slice = [1, 2, 4, 8]
dist = [0.25, 0.5, 0.75]
batchSize = [1, 20, 40]

for k in range(0, 15):
        t = []
        njob = 1000
        fname = "workload/load" + str(k) + ".txt"
        f = open(fname, "w")
        t = np.random.poisson(3.0, njob)
        for i in range(1, len(t)):
            t[i] = t[i-1] + t[i]
        for i in range(0, len(t)):
            t[i] = t[i - i % batchSize[k//5] + batchSize[k//5] // 2]

        for i in range(njob):
            x = random.random()
            sliceIdx = 3
            if x < dist[0]:
                sliceIdx = 0
            elif x < dist[1]:
                sliceIdx = 1
            elif x < dist[2]:
                sliceIdx = 2
            seq = random.randint(10,100)
            para = random.randint(50,500)
            runtime = []
            for j in slice:
                runtime.append(seq + para // j)
            f.write(str(int(t[i])) + " " + ' '.join(str(rt) for rt in runtime) + " " + str(slice[sliceIdx]) + "\n")

        f.close()