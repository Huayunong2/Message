.PHONY:all
all: client server
client: protobuf_client.cpp request.pb.cc /home/hua/libs/muduo-master/build/muduo/protoc/codec.cc
	g++ -o $@ $^ -I/home/hua/libs/muduo-master/build/muduo/protoc -L/home/hua/libs/muduo-master/build/muduo/protoc -lmuduo_net -lmuduo_base -lpthread -lprotobuf -std=c++11 -lz

server: protobuf_server.cpp request.pb.cc /home/hua/libs/muduo-master/build/muduo/protoc/codec.cc
	g++ -o $@ $^ -I/home/hua/libs/muduo-master/build/muduo/protoc -L/home/hua/libs/muduo-master/build/muduo/protoc -lmuduo_net -lmuduo_base -lpthread -lprotobuf  -std=c++11 -lz

.PHONY:clean

clean:
	rm -rf client server