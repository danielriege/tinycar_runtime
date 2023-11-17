# Tinycar Runtime
Workspace to develop and test the tinycar autonomous driving stack. It features a GUI based on Imgui and [nv](https://github.com/nsch0e/nv) for parameter tuning and visualization.
It is meant to be always WIP and not a production ready solution. Using different backends, neural networks can run on different hardware.

## Usage
You need to clone this repo and init the submodules. After that you can use cmake to build the project. If you're on a Mac and want to use CoreML as a backend, you need to build with Ninja: `cmake -G Ninja ..` 

On a Mac you will also need at least Swift 5.9 and Clang to build the project, since we use Swift/C++ interop to use CoreML.

After building, you can run the binary. The first argument is the path to the model file. If you want to use a video file or image as input, you can pass it as the second argument. If you don't pass a second argument, the program will try to connect to the tinycar UDP camera stream. 

You can also specifically define the image provider and the NN runtime by setting the corresponding environment variables. See below for a list of all env variables.

Example:
```
COREML=1 ./tinycar_runtime ../debug_files/vgg.mlpackage ../debug_files/knuff1.mp4
```

## List of env variables
Options are set if value equals 1
### Image Provider
- `TINYCAR`: using tinycar UDP camera stream
- `FILE`: using image or video file (Automatically set if second arg is given)
  
### NN Runtime
- `COREML`: using coreml runtime (requires macOS system with at least Swift 5.9)