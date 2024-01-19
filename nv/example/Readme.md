# nv_example

You have to provide the path to Dear ImGui in CMakeLists.txt.
```
set(IMGUI_DIR imgui/)
```

Or you can just clone it into here:

```bash
git clone https://github.com/ocornut/imgui.git -b docking
```

## Build and Run
```bash
mkdir build && cd build
cmake ..
make
./nv_example IMAGEFILE
```