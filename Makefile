CXX = g++ -O3

SRC_DIR = ./
TARGET = easydb
OBJ = main.o bplus.o

$(TARGET):$(OBJ)
	$(CXX) -o $(TARGET) $(OBJ)
	rm -rf $(OBJ)

main.o:
	$(CXX) -c $(SRC_DIR)main.cpp

bplus.o:
	$(CXX) -c $(SRC_DIR)bplus.cpp

clean:
	rm -rf $(OBJ) $(TARGET)
