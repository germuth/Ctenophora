rm -f binary
g++ -std=c++11 -Wall -g *.cc -o binary

rm -f picture.ppm
./binary >> picture.ppm

rm -f picture.jpg
convert picture.ppm picture.jpg
