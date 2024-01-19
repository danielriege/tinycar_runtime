# NV - Alternative image rendering to opencvs cv::imshow

## Dependencies
 * GLFW3
 * openCV
 * Dear ImGui (docking branch)

## Usage
Please look into the example for usage.

## Features
 * All annotations (lines, arrows, markers, text) can be (de-)activated live without recompiling
   * activation state is saved between sessions
 * annotations are rendered in screenspace, rather than in images space. This makes subpixel precise annotations a easy.
 * annotations support transparency
 * nv::imshow features automatic conversions from different Mat types and can apply colormaps directly.
   * inspector of image shows raw values before conversions
 * simple profiler measuring scope times
 * easy integration of further controls because of Dear ImGui.