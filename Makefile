# Makefile for EE450 Socket Project

client: client.cpp
	g++ -std=c++11 -o client client.cpp

scheduler: scheduler.cpp
	g++ -std=c++11 -o scheduler scheduler.cpp

hospitalA: hospitalA.cpp mapanalyzer.h mapanalyzer.o
	g++ -std=c++11 -o hospitalA hospitalA.cpp mapanalyzer.o

hospitalB: hospitalB.cpp mapanalyzer.h mapanalyzer.o
	g++ -std=c++11 -o hospitalB hospitalB.cpp mapanalyzer.o

hospitalC: hospitalC.cpp mapanalyzer.h mapanalyzer.o
	g++ -std=c++11 -o hospitalC hospitalC.cpp mapanalyzer.o

mapanalyzer.o: mapanalyzer.cpp mapanalyzer.h
	g++ -std=c++11 -c mapanalyzer.cpp

all: client.cpp scheduler.cpp hospitalA.cpp hospitalB.cpp mapanalyzer.cpp
	g++ -std=c++11 -o client client.cpp
	g++ -std=c++11 -o scheduler scheduler.cpp
	g++ -std=c++11 -c mapanalyzer.cpp
	g++ -std=c++11 -o hospitalA hospitalA.cpp mapanalyzer.o
	g++ -std=c++11 -o hospitalB hospitalB.cpp mapanalyzer.o
	g++ -std=c++11 -o hospitalC hospitalC.cpp mapanalyzer.o

clean:
	rm *.o client scheduler hospitalA hospitalB hospitalC