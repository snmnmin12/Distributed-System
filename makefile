all: crsd crc

NetWork.o: NetWork.H NetWork.C util.H util.C
	g++ -c -g  NetWork.C  util.C -lpthread 

crsd: crsd.C NetWork.o util.H util.C
	g++ -g  -o crsd crsd.C NetWork.o  util.C -lpthread

crc: crc.C NetWork.o util.H util.C
	g++ -g -o crc crc.C NetWork.o util.C  -lpthread
