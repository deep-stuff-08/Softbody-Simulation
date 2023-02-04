g++ fpslock.cpp fullscreenquad.cpp beautygui.cpp linuxwindowing.cpp softbodycpu.cpp softbodyopencl.cpp softbodycompute.cpp softbodyvertex.cpp shaderloader.cpp fontrender.cpp opencl.cpp main.cpp -o main.run -lfreetype -lGL -lGLEW -lX11 -lOpenCL -lopenal -lalut -lSOIL
vblank_mode=0 ./main.run
rm main.run