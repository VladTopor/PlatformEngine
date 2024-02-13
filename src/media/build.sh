cd /home/vladislav/CLionProjects/perpg/src
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/vladislav/platform-engine-2d/build/
g++ main.cpp -o ../build/engine.out -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system \
-L/home/vladislav/platform-engine-2d/src/luacpp/build/ \
-I/usr/local/include/LuaCpp -I/home/vladislav/platform-engine-2d/src/luacpp/Souce -I/usr/include/lua5.3/ -lluacpp -llua5.3 -lstdc++
cd ../build
./engine.out
cd ../src
