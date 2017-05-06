# TestopiaClient
Programmatically interfacing Testopia, to e.g. automatically create and update test cases when running CppUTest 

## TestopiaClient executable
The TestopiaClient executable reads CppUTest output from stdin and updates test data in Testopia automatically.

### Compiling TestopiaClient executable on Linux:

Download expat-2.1.0 and ulxmlrpcpp-1.7.5 to folder ~/Downloads from the following URLs:
- http://sf.net/projects/expat/files/expat/2.1.0/expat-2.1.0.tar.gz/download
- http://sf.net/projects/ulxmlrpcpp/files/ulxmlrpcpp/1.7.5/ulxmlrpcpp-1.7.5-src.tar.bz2/download

Extract and build expat
```bash
tar xf ~/Downloads/expat-2.1.0.tar.gz
cd expat-2.1.0
./configure
make
cd ..
```

Extract and prepare ulxmlrpcpp
```bash
tar xf ~/Downloads/ulxmlrpcpp-1.7.5-src.tar.bz2
cd ulxmlrpcpp-1.7.5
CPPFLAGS="-I../expat-2.1.0 -I../expat-2.1.0/lib -DHAVE_EXPAT_CONFIG_H" LDFLAGS=-L../expat-2.1.0/.libs ./configure
cd ..
```

Clone and compile TestopiaClient
```bash
git clone https://github.com/PaulBussmann/TestopiaClient.git
mkdir build-Linux
g++ -o build-Linux/TestopiaClient ./expat-2.1.0/lib/*.c ./ulxmlrpcpp-1.7.5/ulxmlrpcpp/*.cpp ./TestopiaClient/src/TestopiaRpcClient.cpp ./TestopiaClient/src/TestopiaClientMain.cpp  -DXML_STATIC -DULXR_STATIC_LIB -DHAVE_EXPAT_CONFIG_H -DULXR_HAVE_CONFIG_H -ITestopiaClient/include -Iulxmlrpcpp-1.7.5 -Iexpat-2.1.0 -Iexpat-2.1.0/lib -std=c++11 -lpthread
```
# CppUTest
Compile CppUTestTests on Linux
```bash
git clone https://github.com/cpputest/cpputest.git
cd cpputest
apt-get install cmake
cmake CMakeLists.txt
make
```
Execute
```bash
./tests/CppUTest/CppUTestTests -v
cd ..
```

# Example: Live update Testopia from CppUTestTests

Configure Testopia
- Rename "TestProduct" to "CppUTest" at http://<IP>/editproducts.cgi?action=edit&product=TestProduct
- Add plan "CppUTestTests", add build "unspecified" at http://<IP>/page.cgi?id=tr_show_product.html

Execute
```bash
./cpputest/tests/CppUTest/CppUTestTests -v | ./build-Linux/TestopiaClient -product CppUTest -build unspecified -environment --- -planid 1

```
