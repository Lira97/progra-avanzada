Enrique Lira Martinez
A01023351

INSTALLATION

make (The makefile has some modifications regarding the flags since we need to use the threads it is necessary to use the -fopenmp)

EXECUTION (for the execution of the program it is necessary to make use of some flags which receive the arguments in the following way)
    Filter image
    . /test_ppm -i  {name of image file} -o  {name of resulting file} -f  {file with convolution matrix}
