ALL= test_run_shanghai
LIB= -lcurl -lrdkafka -lpthread -lrt  -lssl -lcrypto -ljsoncpp
Include = -I/usr/local/include/librdkafka -I/usr/local/include -I/home/panghao/include 
Flags = -g -O2 -fPIC -Wall -Wsign-compare -Wfloat-equal -Wpointer-arith

objects= tele_task_producer.o tele_task_consumer.o main.o json_data.o CurlWrapper.o   basictypes.o base64.o KafkaWrapper.o

$(ALL) : $(objects)
	g++ -o   $(Iclude) $@ $(objects) $(LIB)
%.o:%.cpp
	g++  -c $(Include)  $< -o $@
.PHONY : clean

clean:
	rm $(objects)

