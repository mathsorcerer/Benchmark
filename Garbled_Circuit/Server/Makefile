Server: Server.cpp
	g++ *.cpp -o Server -I../EMP_Toolkit -I/usr/local/include -L/usr/local/lib/ -lzmq -lcrypto -pthread -Wall -funroll-loops -Wno-ignored-attributes -Wno-unused-result -maes -msse4.2 -std=c++11 -O3

clean:
	rm -f Server
