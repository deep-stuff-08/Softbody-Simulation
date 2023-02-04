![GitHub](https://img.shields.io/github/license/deep-stuff-08/Softbody-Simulation?style=plastic) ![GitHub](https://img.shields.io/badge/platforms-linux-success?style=plastic) ![GitHub](https://img.shields.io/badge/dependency-FreeType_|_GLEW_|_X11_|_OpenCL_|_OpenAL_|_ALUT_|_SOIL-orange?style=plastic)

# Softbody-Simulation

## Introduction

Softbodies are objects mostly used to render organic bodies such as foods or organs and soft materials such as rubber or plastics. The program uses a Spring - Mass System which assumes each point like a particle with 'm' mass connected to neighboring points with springs. These springs use hooke's law to calculate force which keeps the body intact but also flexible.

## Effects Used

1. Gravity
1. 3D Collision Detection and Resolution
1. Spring-Mass System
1. Phong Lighting
1. Mouse Picking
1. FPS Calculation

## Requirements
To run the code you must have a working installation of:

1. FreeType 
1. GLEW
1. X11 Devel
1. OpenCL 
1. OpenAL
1. ALUT
1. SOIL

## Run Instructions

Run the bash script 'run.sh' and if all dependencies are in place the code should run automatically

```bash
$ run.sh
```

## Limitations

Currently the codes are only work on Linux as the windowing is programmed in X11 but could theortically be run on any Operating System with a working X11 wrapper or implementatiion like WSL on Windows.

## Outputs
Video of the following can be found [here](https://youtu.be/v42nMlAFVdc).
