import numpy as np
import random

T = 4000
slice = [1, 2, 4, 8]
dist = [0.1, 0.35, 0.8, 1.0]
batchSize = 1

for k in range(0, 5):
    t = []
    # njob = 2000 + k * 300
    njob = 5000 + k * 300
    fname = "workload/load" + str(k) + ".txt"
    f = open(fname, "w")
    for i in range(njob // batchSize + 1):
        t.append(random.randint(0,T))
        # t.append(0)

    t.sort()

    for i in range(njob):
        x = random.random()
        sliceIdx = 3
        if x < dist[0]:
            sliceIdx = 0
        elif x < dist[1]:
            sliceIdx = 1
        elif x < dist[2]:
            sliceIdx = 2
        # sliceIdx = random.randint(0,3)
        seq = random.randint(2,20)
        para = random.randint(10,100)
        runtime = []
        for j in slice:
            # runtime.append((seq + para) // j)
            runtime.append(seq + para // j)
        f.write(str(t[i // batchSize]) + " " + ' '.join(str(rt) for rt in runtime) + " " + str(slice[sliceIdx]) + "\n")

    f.close()