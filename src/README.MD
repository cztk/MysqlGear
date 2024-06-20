# Dependencies
## libzdb
 http://www.tildeslash.com/libzdb/ for database connection pooling
 ####if build fails, try git clone https://bitbucket.org/tildeslash/libzdb.git
 ####or dependencies/tildeslash-libzdb-ed5c65ddfda1.tar.gz
 ####or
``` 
sudo apt install libssl-dev  libmariadb-dev-compat re2c libtool libacl1
./bootstrap
./configure --enable-optimized --enable-protected --enable-openssl --with-mysql --enable-static=no
make
sudo make install
sudo ldconfig /usr/local/lib
```

# Config options

## network
gear_server_rcv_timeout_sec
default: 3 

gear_server_rcv_timeout_usec
default: 0

gear_server_snd_timeout_sec
default: 3 

gear_server_snd_timeout_usec
default: 0

gear_server_max_connections
default: 20


## N_STATUS
```
int8_t length
followed by csv data
inputMessageQueue ( sql queries );
max database connections;
active database connections;
net input thread count;
net input tasks running;
net input tasks queued;
net input tasks total;
sql query tasks count;
sql query tasks running;
sql query tasks queued;
sql query tasks total;
```