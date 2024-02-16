CXX = g++
CXXFLAGS = -O3 -Wall

all: main

main: main.o a100.o job.o cluster.o logger.o instance.o
	${CXX} ${CXXFLAGS} main.o a100.o job.o cluster.o logger.o instance.o -o main

main.o: main.cpp cluster.h a100.h job.h logger.h instance.h util.h
	${CXX} ${CXXFLAGS} -c main.cpp 

a100.o: a100.cpp a100.h util.h instance.h job.h
	${CXX} ${CXXFLAGS} -c a100.cpp

job.o: job.cpp job.h util.h
	${CXX} ${CXXFLAGS} -c job.cpp

cluster.o: cluster.cpp cluster.h util.h a100.h job.h logger.h instance.h
	${CXX} ${CXXFLAGS} -c cluster.cpp

logger.o: logger.cpp logger.h job.h util.h
	${CXX} ${CXXFLAGS} -c logger.cpp

instance.o: instance.cpp instance.h util.h
	${CXX} ${CXXFLAGS} -c instance.cpp

clean:
	rm main *.o