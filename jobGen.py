import numpy as np
import random

T = 4000
slice = [1, 2, 4, 8]
batchSize = 1

for k in range(0, 5):
    t = []
    # njob = 2000 + k * 300
    njob = 4000 + k * 300
    fname = "workload/load" + str(k) + ".txt"
    f = open(fname, "w")
    for i in range(njob // batchSize + 1):
        t.append(random.randint(0,T))
        # t.append(0)

    t.sort()

    for i in range(njob):
        sliceIdx = random.randint(0,1)
        # rt = random.randint(10, 200)
        # ratio = 0.0
        # seq = (int)(rt * ratio)
        # para = rt - seq
        seq = random.randint(4,40)
        para = random.randint(8,80)
        runtime = []
        for j in slice:
            runtime.append(seq + para // j)
        f.write(str(t[i // batchSize]) + " " + ' '.join(str(rt) for rt in runtime) + " " + str(slice[sliceIdx]) + "\n")

    f.close()