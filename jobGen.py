import numpy as np
import random

slice = [1, 2, 4, 8]
dist = [0.25, 0.5, 0.75]
batchSize = 1

for k in range(0, 5):
    t = []
    njob = 1000
    fname = "workload/load" + str(k) + ".txt"
    f = open(fname, "w")
    # for i in range(njob // 50):
    #     lam = 8 - k / 2 + (random.random() - 0.5) * 4
        # t = np.concatenate((t, np.random.poisson(lam, 100)))
    t = np.random.poisson(50 - k / 2, njob)
    for i in range(1, len(t)):
        t[i] = t[i-1] + t[i]

    cnt = 0
    for i in range(njob):
        if cnt == 50:
            for d in range(3):
                dist[d] = random.random()
            dist.sort()
            # print(dist)
            cnt = 0
        cnt += 1
        x = random.random()
        sliceIdx = 3
        if x < dist[0]:
            sliceIdx = 0
        elif x < dist[1]:
            sliceIdx = 1
        elif x < dist[2]:
            sliceIdx = 2
        seq = random.randint(2,200)
        para = random.randint(10,1000)
        runtime = []
        for j in slice:
            # runtime.append((seq + para) // j)
            runtime.append(seq + para // j)
        f.write(str(int(t[i // batchSize])) + " " + ' '.join(str(rt) for rt in runtime) + " " + str(slice[sliceIdx]) + "\n")

    f.close()