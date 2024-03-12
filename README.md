a100-cluster-simulator
===

## Introduction
我的碩士論文研究題目是針對[MIG GPU](https://docs.nvidia.com/datacenter/tesla/mig-user-guide/index.html)提出工作排程及資源分配方法。為此我設計了這個 a100 cluster simulator。
此 project 包含了 workload generator, cluster simulator 以及 result analyzer。

## Compilation
```bash
./make
```
詳見 Makefile

## Execution
- 執行完整實驗的作法請參考 defaultRun.sh
- 以下為個別的步驟:
1. Generate workload
```bash
./python genJob.py $i $j
```
  - i: # workload configs
  - j: # testcases for each config

2. Simulated experiment
```bash
./main.exe $NGPU $i $j $k
```
  - NGPU: cluster 內 GPU 的數量
  - i: workload config ID
  - j: testcase ID
  - k: algorithm ID

3. Result analyze
```bash
./python result.py $i $j
```
  - i: # workload configs
  - j: # testcases for each config
