cmake_minimum_required(VERSION 2.8)
aux_source_directory(. src)
include_directories(/usr/local/include /home/panghao/include /home/panghao/include/librdkafka)
link_directories(/usr/local/lib /home/panghao/libs)
add_executable(kun_run_shanghai ${src})
target_link_libraries(kun_run_shanghai curl glog rdkafka jsoncpp crypto pthread)
