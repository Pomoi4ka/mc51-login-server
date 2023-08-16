CC=cc
CXX=c++

LIBS_CFLAGS!=pkg-config --cflags openssl
CFLAGS=-Wall -Wextra -pedantic -Wno-deprecated-declarations -pipe -ggdb ${LIBS_CFLAGS}
LIBS!=pkg-config --libs openssl

build: src/main.o src/BufStream.o src/DataClasses.o src/Crypter.o codegen/gen/Packets.o
	$(CXX) $(LIBS) -o $@ src/main.o src/BufStream.o src/Crypter.o codegen/gen/Packets.o src/DataClasses.o

src/main.o: src/main.cpp src/BufStream.hpp src/Crypter.hpp codegen/gen/Packets.hpp
	$(CXX) $(CFLAGS) -c -o $@ src/main.cpp

src/BufStream.o: src/BufStream.cpp src/BufStream.hpp src/Crypter.hpp
	$(CXX) $(CFLAGS) -c -o $@ src/BufStream.cpp

src/Crypter.o: src/Crypter.cpp src/Crypter.hpp
	$(CXX) $(CFLAGS) -c -o $@ src/Crypter.cpp

src/DataClasses.o: src/DataClasses.cpp src/DataClasses.hpp src/BufStream.hpp
	$(CXX) $(CFLAGS) -c -o $@ src/DataClasses.cpp

codegen/gen/Packets.o: codegen/gen/Packets.cpp codegen/gen/Packets.hpp src/BufStream.hpp
	$(CXX) $(CFLAGS) -Wno-unused-parameter -c -o $@ codegen/gen/Packets.cpp -I./codegen/gen -I./src

codegen/gen/Packets.cpp codegen/gen/Packets.hpp: codegen/packets codegen/gen
	codegen/packets -declo codegen/gen/Packets.hpp -defo codegen/gen/Packets.cpp

codegen/packets: codegen/packets.c
	$(CC) $(CFLAGS) -Wno-missing-field-initializers -Wno-format-overflow -o $@ codegen/packets.c

codegen/gen:
	mkdir -p $@
