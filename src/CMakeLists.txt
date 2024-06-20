cmake_minimum_required(VERSION 3.16.3)
project(MysqlGear)

option(RELEASE_BUILD "Enable release build" OFF) #OFF by default

set(CMAKE_CXX_STANDARD 20)
if (CMAKE_BUILD_TYPE MATCHES Debug)
    message(" ")
    option(RELEASE_BUILD "Enable release build" OFF) #OFF by default
    message("CMAKE IN DEBUG MODE")
    message(" ")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    message(" ")
    option(RELEASE_BUILD "Enable release build" ON) #OFF by default
    message("CMAKE IN RELEASE MODE")
    message(" ")
endif ()


include_directories("/usr/local/include/zdb")

if (RELEASE_BUILD)
    SET(GCC_COVERAGE_COMPILE_FLAGS "-fno-rtti -Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -fomit-frame-pointer -Wall -O2 -ftree-loop-vectorize -flto")
    SET(GCC_COVERAGE_LINK_FLAGS "-pthread -flto")
else ()
    SET(GCC_COVERAGE_COMPILE_FLAGS "-fno-rtti -fexceptions -Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -Wall -O0 -g3 -D_FORTIFY_SOURCE=1 -fasynchronous-unwind-tables -Werror=format-security -DDEBUG=1 -D_DEBUG=1 ")
    SET(GCC_COVERAGE_LINK_FLAGS "-pthread")
endif (RELEASE_BUILD)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

add_executable(MysqlGear main.cpp utils/daemonize/daemonize.h utils/daemonize/daemonize.cpp config.h mysqlserver.h input/InputHandler.h input/InputHandler.cpp input/inputs/GearServer.cpp input/inputs/GearServer.h utils/tcpclientserver/Server.cpp utils/tcpclientserver/Server.h input/inputs/GearStreamClientHandler.cpp input/inputs/GearStreamClientHandler.h utils/lockedqueue/LockedQueue.h input/inputs/GearInputMessageTypes.h output/OutputHandler.cpp output/OutputHandler.h utils/threadpool/thread_pool.hpp MysqlGearQueueEntry.h utils/libzdb/LibzdbHandler.cpp utils/libzdb/LibzdbHandler.h utils/threadpool/ThreadpoolHandler.cpp utils/threadpool/ThreadpoolHandler.h)
target_link_libraries(MysqlGear zdb)

set(CMAKE_INSTALL_PREFIX /usr/local/bin/)

install(CODE "execute_process(COMMAND mkdir /etc/mysqlgear/)")
install(CODE "execute_process(COMMAND mkdir /usr/local/run/mysqlgeard)")
install(CODE "execute_process(COMMAND mkdir /var/log/mysqlgear)")

install(CODE "execute_process(COMMAND touch /var/log/mysqlgear/info.log)")
install(CODE "execute_process(COMMAND touch /var/log/mysqlgear/error.log)")
install(CODE "execute_process(COMMAND touch /var/log/mysqlgear/alert.log)")
install(CODE "execute_process(COMMAND chown syslog:adm /var/log/mysqlgear/*.log)")

install(FILES files/server.conf DESTINATION /etc/mysqlgear/)
install(FILES files/rsyslog.d/01-mysqlgeard.conf DESTINATION /etc/rsyslog.d/)
install(FILES files/logrotate.d/mysqlgear DESTINATION /etc/logrotate.d/)
install(FILES files/mysqlgear.service DESTINATION /etc/systemd/system/)
install(TARGETS MysqlGear RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

install(CODE "execute_process(COMMAND systemctl restart rsyslog)")
install(CODE "execute_process(COMMAND systemctl daemon-reload)")
install(CODE "execute_process(COMMAND systemctl enable mysqlgear)")