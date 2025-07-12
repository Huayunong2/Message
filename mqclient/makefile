.PHONYT:public_client consume_client
all: public_client consume_client
public_client:public_client.cc ../mqcommon/mq_msg.pb.cc ../mqcommon/mq_proto.pb.cc ../mqthird/muduo/protoc/codec.cc
	g++ -o $@ $^ -std=c++14 -lprotobuf -pthread -lmuduo_net -lmuduo_base -lz

consume_client:consume_client.cc ../mqcommon/mq_msg.pb.cc ../mqcommon/mq_proto.pb.cc ../mqthird/muduo/protoc/codec.cc
	g++ -o $@ $^ -std=c++14 -lprotobuf -pthread -lmuduo_net -lmuduo_base -lz

.PHONY:clean
clean:
	rm -rf public_client consume_client