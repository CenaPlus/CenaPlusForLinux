clang -O2 -Wall -c -o CenaPlus.Sandbox.o CenaPlus.Sandbox.cpp -g
g++ -o MLE ./Tests/MLE.cpp
clang -O2 -Wall -o TLE1 ./Tests/TLE1.cpp
clang -O2 -Wall -o TLE2 ./Tests/TLE2.cpp -g
clang -O2 -Wall -o ReturnValue ./Tests/ReturnValue.cpp
g++ -o Danger ./Tests/Danger.cpp

g++ -O2 -Wall -o doTest ./CenaPlus.Sandbox.Test.cpp CenaPlus.Sandbox.o -pthread -std=c++11 -lrt -lseccomp -g
./doTest
