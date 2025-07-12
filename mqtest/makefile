.PHONY:mq_connectiontest

mq_connectiontest:mq_connectiontest.cc ../mqcommon/mq_msg.pb.cc ../mqcommon/mq_proto.pb.cc
	g++ -o $@ $^ -std=c++14 -lgtest -lprotobuf -lsqlite3

channeltest:mq_channeltest.cc ../mqcommon/mq_msg.pb.cc ../mqcommon/mq_proto.pb.cc
	g++ -o $@ $^ -std=c++14 -lgtest -lprotobuf -lsqlite3

consumertest:mq_consumertest.cc ../mqcommon/mq_msg.pb.cc
	g++ -o $@ $^ -std=c++14 -lgtest -lprotobuf -lsqlite3

hosttest:mq_hosttest.cc ../mqcommon/mq_msg.pb.cc
	g++ -o $@ $^ -std=c++14 -lgtest -lprotobuf -lsqlite3

msgtest:mq_msgtest.cc ../mqcommon/mq_msg.pb.cc
	g++ -o $@ $^ -std=c++14 -lgtest -lprotobuf -lsqlite3

bindingtest:mq_bindingtest.cc
	g++ -o $@ $^ -std=c++14 -lgtest -lprotobuf -lsqlite3
	
queuetest:mq_queuetest.cc
	g++ -o $@ $^ -std=c++14 -lgtest -lprotobuf -lsqlite3

exchangetest:mq_exchangetest.cc
	g++ -o $@ $^ -std=c++14 -lgtest -lprotobuf -lsqlite3

filetest:mq_filetest.cc
	g++ -o $@ $^ -std=c++11

.PHONY:clean
clean:
	rm -rf filetest bindingtest exchangetest queuetest msgtest hosttest routertest consumertest channeltest mq_connectiontest