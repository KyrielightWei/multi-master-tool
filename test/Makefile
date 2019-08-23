CXXFLAGS += -std=c++11 

RELATIVE_PREFIX = ../

PERFORMANCE_LIB_HEADER = $(RELATIVE_PREFIX)src/performance_statistics/perfor_statistics.h
PERFORMANCE_API_HEADER = $(RELATIVE_PREFIX)src/include/performance.h
PERFORMANCE_LIB_SOURCE = $(RELATIVE_PREFIX)src/performance_statistics/perfor_statistics.cc
PERFORMANCE_API_SOURCE = $(RELATIVE_PREFIX)src/performance_statistics/perfor_c_api.c
PERFORMANCE_API_OBJECT = target/perfor_c_api.o
PERFORMANCE_LIB_OBJECT_THREAD = target/perfor_statistics_thread.o
PERFORMANCE_LIB_OBJECT_ONETIME = target/perfor_statistics_onetime.o
PERFORMANCE_LIB_STATIC_THREAD = target/libPerformanceThread.a
PERFORMANCE_LIB_STATIC_ONETIME = target/libPerformanceOnetime.a


.PHONY: all clean

all: performance_test

performance_test:performance_statistics_lib
	gcc -o target/test_performance_onetime performance_test.c -L./target -lPerformanceOnetime -lstdc++ 
	gcc -o target/test_performance_thread performance_test.c -L./target -lPerformanceThread -lstdc++  -pthread

performance_statistics_lib:  performance_statistics_thread_object performance_statistics_onetime_object 
	ar -crv $(PERFORMANCE_LIB_STATIC_THREAD) $(PERFORMANCE_LIB_OBJECT_THREAD) 
	ar -crv $(PERFORMANCE_LIB_STATIC_ONETIME) $(PERFORMANCE_LIB_OBJECT_ONETIME) 
	@echo "Build performance_statistics"

performance_statistics_thread_object: target_dir
	sed -i "s/#define FLUSH_MODE .*/#define FLUSH_MODE THREAD_FLUSH/g" ${PERFORMANCE_LIB_HEADER}
	g++ -c $(PERFORMANCE_LIB_SOURCE) -o $(PERFORMANCE_LIB_OBJECT_THREAD) ${CXXFLAGS}

performance_statistics_onetime_object: target_dir
	sed -i "s/#define FLUSH_MODE .*/#define FLUSH_MODE ONETIME_FLUSH/g" ${PERFORMANCE_LIB_HEADER}
	g++ -c $(PERFORMANCE_LIB_SOURCE) -o $(PERFORMANCE_LIB_OBJECT_ONETIME) ${CXXFLAGS}

target_dir :
	if [ ! -d target]; then  \
　　	mkdir target;  \
	fi  

clean:
	rm -rf target