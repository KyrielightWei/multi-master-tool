CXXFLAGS += -std=c++11 
TEST_LIB_FLAGS = -L./target  -lstdc++  -pthread

RELATIVE_PREFIX = ../

.PHONY: all clean

all: performance_test log_flush_test config_io_test

####### Performance tool ##########
PERFORMANCE_LIB_HEADER = $(RELATIVE_PREFIX)src/performance_statistics/perfor_statistics.h
PERFORMANCE_API_HEADER = $(RELATIVE_PREFIX)src/include/performance.h
PERFORMANCE_LIB_SOURCE = $(RELATIVE_PREFIX)src/performance_statistics/perfor_statistics.cc
PERFORMANCE_API_SOURCE = $(RELATIVE_PREFIX)src/performance_statistics/perfor_c_api.c
PERFORMANCE_API_OBJECT = target/perfor_c_api.o
PERFORMANCE_LIB_OBJECT_THREAD = target/perfor_statistics_thread.o
PERFORMANCE_LIB_OBJECT_ONETIME = target/perfor_statistics_onetime.o
PERFORMANCE_LIB_STATIC_THREAD = target/libPerformanceThread.a
PERFORMANCE_LIB_STATIC_ONETIME = target/libPerformanceOnetime.a
performance_test:performance_statistics_lib
	gcc -o target/test_performance_onetime performance_test.c  $(TEST_LIB_FLAGS) -lPerformanceOnetime
	gcc -o target/test_performance_thread performance_test.c $(TEST_LIB_FLAGS) -lPerformanceThread 

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

####### Log flush tool ##########
LOG_FLUSH_SOURCE =  $(RELATIVE_PREFIX)src/log_flush/log_flush.cc
LOG_FLUSH_OBJECT = target/log_flush.o
LOG_FLUSH_STATIC_LIB = target/libLogFlush.a

log_flush_object: target_dir
	g++ -c $(LOG_FLUSH_SOURCE) -o $(LOG_FLUSH_OBJECT) $(CXXFLAGS)

log_flush_lib: log_flush_object
	ar -crv $(LOG_FLUSH_STATIC_LIB) $(LOG_FLUSH_OBJECT)
	@echo "Build log_flush"

log_flush_test:log_flush_lib
	gcc -o target/test_log_flush log_flush_test.c $(TEST_LIB_FLAGS) -lLogFlush


###### Config IO #########
CONFIG_SOURCE = $(RELATIVE_PREFIX)src/configIO/config_io.cc
CONFIG_OBJECT = target/config_io.o
CONFIG_STATIC_LIB = target/libConfigIo.a

config_io_object: target_dir
	g++ -c $(CONFIG_SOURCE) -o $(CONFIG_OBJECT) $(CXXFLAGS)

config_io_lib: config_io_object
	ar -crv $(CONFIG_STATIC_LIB) $(CONFIG_OBJECT)
	@echo "Build log_flush"

config_io_test:config_io_lib
	gcc -o target/test_config_io config_io_test.c $(TEST_LIB_FLAGS) -lConfigIo


####### common ###########
target_dir :
	if [ ! -d target ]; then  mkdir target;  fi;  

clean:
	rm -rf target