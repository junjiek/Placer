CXX = g++
#OPTFLAG = -O3 -Wall -W -Wconversion -Wcast-qual -g

OPTFLAG = -O3 -g -gstabs+

CFLAG = -I./include \
	    -I./include/oa\
		-I/usr/local/include -I/usr/include\
		-I./eigen

OA_DIR:=./src/oa/lib/linux_rhel30_gcc411_64/opt
OA_SHARED_DIR:=$(OA_DIR)/shared

BASE_LIB = \
		   -L$(OA_DIR) \
		   -loaBase -loaDM -loaDesign -loaCommon -loaPlugIn -loaRQXYTree -loaTech -loaTcl \
		   -ldl \
		   -lstdc++ \
		   -lm \
		   -ltcl \
		   -ltk

LFLAG = $(BASE_LIB)


TARGET= placer

SRCS =$(wildcard src/*.cpp)

OBJS = $(SRCS:src/%.cpp=obj/%.o)




all : $(OBJS)
	$(CXX) $(OPTFLAG) $(OBJS) $(LFLAG) -o  $(TARGET) 

obj/%.o : ./src/%.cpp 
	$(CXX) $(OPTFLAG) $(CFLAG) -c $^ -o $@

clean:
	rm -f ./obj/* $(TARGET) *~ ./src/*~/ ./include/*~







