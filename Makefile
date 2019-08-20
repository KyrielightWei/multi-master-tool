# complie lib into target folder

CXXFLAGS += -std=c++11
LDFLAGS += -L/usr/local/lib `pkg-config --libs protobuf grpc++ grpc` -Wl,--as-needed -lpthread -ldl

.PHONY: all 

all : consistency_check performance_statistics
	@echo "Build Success"

consistency_check: target_dir 
	@echo "Build consistency_check"

performance_statistics: target_dir 
	@echo "Build performance_statistics"

target_dir : 
	mkdir target