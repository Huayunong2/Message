.PHONYT:mq_server
mq_server:mq_server.cc ../mqcommon/mq_msg.pb.cc ../mqcommon/mq_proto.pb.cc ../mqthird/muduo/protoc/codec.cc
	g++ -o $@ $^ -std=c++14 -lgtest -lprotobuf -lsqlite3 -pthread -lmuduo_net -lmuduo_base -lz

.PHONY:clean
clean:
	rm -rf mq_server