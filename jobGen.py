import numpy as np
import random

T = 4000
slice = [1, 2, 4, 8]
dist = [0.25, 0.5, 0.75, 1.0]
# dist = [0.5, 1.0, 1.0, 1.0]
batchSize = 1

for k in range(0, 5):
    t = []
    njob = 1000
    fname = "workload/load" + str(k) + ".txt"
    f = open(fname, "w")
    # t = np.random.poisson(7 - k / 2, njob)
    t = np.random.poisson(10 - k / 2, njob)
    for i in range(1, len(t)):
        t[i] = t[i-1] + t[i]

    for i in range(njob):
        x = random.random()
        sliceIdx = 3
        if x < dist[0]:
            sliceIdx = 0
        elif x < dist[1]:
            sliceIdx = 1
        elif x < dist[2]:
            sliceIdx = 2
        seq = random.randint(20,200)
        para = random.randint(100,1000)
        runtime = []
        for j in slice:
            # runtime.append((seq + para) // j)
            runtime.append(seq + para // j)
        f.write(str(t[i // batchSize]) + " " + ' '.join(str(rt) for rt in runtime) + " " + str(slice[sliceIdx]) + "\n")

    f.close()