echo [BUILDING]

g++ -c *.cpp `libpng-config --cflags` 
g++ -o hw5 *.o `libpng-config --ldflags --libs`
rm *.o

echo [DONE]
