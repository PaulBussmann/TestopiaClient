# TestopiaClient
Programmatically interfacing Testopia, to e.g. automatically create and update test cases when running CppUTest 

## TestopiaClient executable
The TestopiaClient executable reads CppUTest output from stdin and updates test data in Testopia automatically.

### Compiling TestopiaClient executable on Linux:

Needs expat-2.1.0 (http://sf.net/projects/expat) and ulxmlrpcpp-1.7.5 (http://sf.net/projects/ulxmlrpcpp) in ~/Downloads:

#### Extract and build expat
```bash
tar xf ~/Downloads/expat-2.1.0.tar.gz
cd expat-2.1.0
./configure
make
cd ..
```

#### Extract and prepare ulxmlrpcpp
```bash
tar xf ~/Downloads/ulxmlrpcpp-1.7.5-src.tar.bz2
cd ulxmlrpcpp-1.7.5
CPPFLAGS="-I../expat-2.1.0 -I../expat-2.1.0/lib -DHAVE_EXPAT_CONFIG_H" LDFLAGS=-L../expat-2.1.0/.libs ./configure
cd ..
```

#### Clone and compile TestopiaClient
```bash
git clone https://github.com/PaulBussmann/TestopiaClient.git
g++ -o build-Linux/TestopiaClient ./expat-2.1.0/lib/*.c ./ulxmlrpcpp-1.7.5/ulxmlrpcpp/*.cpp ./TestopiaClient/src/TestopiaRpcClient.cpp ./TestopiaClient/src/TestopiaClientMain.cpp  -DXML_STATIC -DULXR_STATIC_LIB -DHAVE_EXPAT_CONFIG_H -DULXR_HAVE_CONFIG_H -ITestopiaClient/include -Iulxmlrpcpp-1.7.5 -Iexpat-2.1.0 -Iexpat-2.1.0/lib -std=c++11 -lpthread
```
# CppUTest
## CppUTestTests
### Compile on Linux
```bash
git clone https://github.com/cpputest/cpputest.git
cd cpputest
apt-get install cmake
cmake CMakeLists.txt
make
```
### Run on Linux
```bash
./tests/CppUTest/CppUTestTests -v
```
