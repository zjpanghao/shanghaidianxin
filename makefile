ALL= kun_run_shanghai
LIB= -lcurl -lrdkafka -lpthread -lrt  -lssl -lcrypto -ljsoncpp -lglog
Include = -I/usr/local/include/librdkafka -I/usr/local/include -I/home/panghao/include 
Flags = -g -O2 -fPIC -Wall -Wsign-compare -Wfloat-equal -Wpointer-arith

objects= tele_mat.o tele_task_producer.o tele_task_consumer.o main.o json_data.o CurlWrapper.o    base64.o KafkaWrapper.o

$(ALL) : $(objects)
	g++  -g -o    $(Iclude) $@ $(objects) $(LIB)
%.o:%.cpp
	g++ $(Flags) -c $(Include)  $< -o $@
.PHONY : clean

clean:
	rm $(objects)

