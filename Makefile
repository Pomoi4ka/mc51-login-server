CC=cc
CXX=c++

CFLAGS=-Wall -Wextra -pedantic -Wno-deprecated-declarations -pipe -ggdb `pkg-config --cflags openssl`
LIBS!=pkg-config --libs openssl

build: src/main.o src/BufStream.o src/Crypter.o
	$(CXX) $(LIBS) -o $@ src/main.o src/BufStream.o src/Crypter.o

src/main.o: src/main.cpp src/BufStream.hpp src/Crypter.hpp
	$(CXX) $(CFLAGS) -c -o $@ src/main.cpp

src/BufStream.o: src/BufStream.cpp src/BufStream.hpp src/Crypter.hpp
	$(CXX) $(CFLAGS) -c -o $@ src/BufStream.cpp

src/Crypter.o: src/Crypter.cpp src/Crypter.hpp
	$(CXX) $(CFLAGS) -c -o $@ src/Crypter.cpp
