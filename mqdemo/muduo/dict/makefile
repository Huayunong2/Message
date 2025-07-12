.PHONY:all
all:dict_client dict_server

dict_client:dict_client.cpp
	g++ -o $@  $^ -lmuduo_net -lmuduo_base -lpthread

dict_server:dict_server.cpp
	g++ -o $@  $^ -lmuduo_net -lmuduo_base -lpthread
	
.PHONY:clean
clean:
	rm -rf dict_server dict_client