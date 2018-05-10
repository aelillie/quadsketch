qs: qs.cpp
	g++ qs.cpp -o qs -O3 -Wall -std=c++11 -march=native -I ../external/simple-serializer -I ../external/eigen -lboost_program_options

qsr: qsr.cpp 
	g++ qsr.cpp -o qsr -O3 -Wall -std=c++11 -pthread -march=native -I ../external/simple-serializer -I ../external/eigen -lboost_program_options

grid: grid.cpp
	g++ grid.cpp -o grid -O3 -Wall -std=c++11 -pthread -march=native -I ../external/simple-serializer -I ../external/eigen -lboost_program_options

clean:
	rm qs qsr grid