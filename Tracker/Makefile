DIR_INC = ./include
DIR_SRC = .
DIR_OBJ = ./obj
DIR_BIN = ./bin

<<<<<<< HEAD
DIR_INC_EX = -I$(SEETATECH_HOME)/include -I$(OPENCV_HOME)/include
DIR_LIB_EX = -L$(SEETATECH_HOME)/lib -L$(OPENCV_HOME)/lib
=======
SEETATECH_DIR = /usr/local/seetatech
OPENCV_DIR = /usr/local/opencv3

DIR_INC_EX = -I${SEETATECH_DIR}/include -I${OPENCV_DIR}/include
DIR_LIB_EX = -L/usr/local/seetatech/lib -L/usr/local/opencv3/lib
>>>>>>> c558f46ba6cf7c74630c53a3170a98f6baffeab2
LIB_EX = -lVIPLFaceDetector500 -lopencv_imgproc -lopencv_highgui -lopencv_videoio -lopencv_core

PREPROCESSOR = -DRENDER_WITH_OPENCV

SRC = $(wildcard ${DIR_SRC}/*.cpp ${DIR_SRC}/src/*.cpp ${DIR_SRC}/src/filter/*.cpp)
OBJ = $(patsubst %.cpp,${DIR_OBJ}/%.o,$(notdir ${SRC}))

TARGET = simple_tracker

BIN_TARGET = ${DIR_BIN}/${TARGET}

CC = g++
CFLAGS = -std=c++11 -g -Wall -I. -I${DIR_INC} ${DIR_INC_EX} ${PREPROCESSOR}
LFLAGS = ${DIR_LIB_EX} ${LIB_EX}

${BIN_TARGET}:${OBJ}$
	$(CC) ${OBJ} $(LFLAGS) -o $@

${DIR_OBJ}/%.o:${DIR_SRC}/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@
${DIR_OBJ}/%.o:${DIR_SRC}/src/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@
${DIR_OBJ}/%.o:${DIR_SRC}/src/filter/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	find $(DIR_OBJ) -name *.o -type f -exec rm -rf {} \;

