CURRENT_PATH=.
BUA=bottomUp
TDA=topDown
GEN=generate_Input
HYB=hybrid

SOURCES_BUA=${BUA}.cpp
SOURCES_TDA=${TDA}.cpp
SOURCES_GEN=${GEN}.cpp
SOURCES_HYB=${HYB}.cpp
OBJECTS_BUA=$(SOURCES_BUA:%.cpp=%.o)
OBJECTS_TDA=$(SOURCES_TDA:%.cpp=%.o)
OBJECTS_GEN=$(SOURCES_GEN:%.cpp=%.o)
OBJECTS_HYB=$(SOURCES_HYB:%.cpp=%.o)

CC = g++
INCLDIRS = -I$(CURRENT_PATH) #for boost
CFLAGS = -Wall
CCFLAGS = $(CFLAGS) $(INCLDIRS)
DFLAGS = -std=c++0x -pthread
EFLAGS = $(CCFLAGS) $(DFLAGS)

all: compile_bua compile_tdA compile_gen compile_hyb

compile_bua: $(OBJECTS_BUA) 
	$(CC) -o $(BUA) $(OBJECTS_BUA:%=$(CURRENT_PATH)/obj/%)
	
$(OBJECTS_BUA): %.o: %.cpp
	$(CC) -c $(CCFLAGS) $< -o $(CURRENT_PATH)/obj/$@
	
compile_tdA: $(OBJECTS_TDA) 
	$(CC) -o $(TDA) $(OBJECTS_TDA:%=$(CURRENT_PATH)/obj/%)
	
$(OBJECTS_TDA): %.o: %.cpp
	$(CC) -c $(CCFLAGS) $< -o $(CURRENT_PATH)/obj/$@	

compile_hyb: $(OBJECTS_HYB) 
	$(CC) -o $(HYB) $(DFLAGS) $(OBJECTS_HYB:%=$(CURRENT_PATH)/obj/%)
	
$(OBJECTS_HYB): %.o: %.cpp
	$(CC) -c $(EFLAGS) $< -o $(CURRENT_PATH)/obj/$@	

compile_gen: $(OBJECTS_GEN) 
	$(CC) -o $(GEN) $(OBJECTS_GEN:%=$(CURRENT_PATH)/obj/%)
	
$(OBJECTS_GEN): %.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $(CURRENT_PATH)/obj/$@


clean:
	rm -f obj/*.o
	rm ${BUA}
	rm ${TDA}
	rm ${GEN}
	rm ${HYB}

