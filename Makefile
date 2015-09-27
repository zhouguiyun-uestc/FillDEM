fillDEM: dem.o main.o utils.o FillDEM_Zhou-Direct.o FillDEM_Zhou-OnePass.o FillDEM_Zhou-TwoPass.o
	g++ dem.o main.o utils.o FillDEM_Zhou-Direct.o FillDEM_Zhou-OnePass.o FillDEM_Zhou-TwoPass.o -lgdal --std=c++11 -fpermissive -o fillDEM

dem.o: dem.h utils.h dem.cpp
	g++ -c dem.cpp --std=c++11 -fpermissive
main.o: dem.h Node.h utils.h FillDEM_Zhou-Direct.cpp FillDEM_Zhou-OnePass.cpp FillDEM_Zhou-TwoPass.cpp main.cpp
	g++ -c main.cpp --std=c++11 -fpermissive
utils.o: dem.h utils.h utils.cpp
	g++ -c utils.cpp --std=c++11 -fpermissive
FillDEM_Zhou-Direct.o: dem.h Node.h utils.h FillDEM_Zhou-Direct.cpp
	g++ -c FillDEM_Zhou-Direct.cpp --std=c++11 -fpermissive
FillDEM_Zhou-OnePass.o: dem.h Node.h utils.h FillDEM_Zhou-OnePass.cpp
	g++ -c FillDEM_Zhou-OnePass.cpp --std=c++11 -fpermissive
FillDEM_Zhou-TwoPass.o: dem.h Node.h utils.h FillDEM_Zhou-TwoPass.cpp
	g++ -c FillDEM_Zhou-TwoPass.cpp --std=c++11 -fpermissive

clean:
	@echo "cleanning project"
	-rm fillDEM *.o
	@echo "clean completed"
.PHONY: clean

