build: 
	g++ -std=c++11 clk.cpp -o clock.out
	g++ -std=c++11 scheduler.cpp -o sch.out
	g++ -std=c++11 FilesGenerator.cpp -o Files.out
	g++ -std=c++11 processGenerator.cpp -o main.out
	g++ -std=c++11 process.cpp -o process.out
              

clean:
	rm -f *.out  processes.txt
	ipcrm -M 300
	ipcrm -Q 777
	@echo "All Items Cleared"

all: clean build

run:
	./main.out
