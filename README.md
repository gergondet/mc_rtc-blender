mc_rtc-blender
==

A Blender addon for working with mc_rtc

Initially based on [BlenderImGui] for the ImGui/Blender integration

Pre-requisites
--

- [Blender] >= 2.8
- [Python] development files (library/header) matching Blender's embedded Python `MAJOR.MINOR` version
- [mc\_rtc]

Installation
--

(Substitute 2.93 with your Blender version in those commands)

1. Clone into `~/.config/blender/2.93/scripts/addons (or `%APPDATA%/blender/2.93/scripts/addons` on Windows)
2. Create a build folder and run CMake, e.g. `cmake ~/.config/blender/2.93/scripts/addons/mc_rtc-blender -DCMAKE_BUILD_TYPE=RelWithDebInfo`
3. Make sure the Python version found here matches the Blender version, on the first CMake run you should see something similar to `-- Found Python3: /usr/bin/python3.9 (found version "3.9.6") found components: Interpreter Development`, if the version does not match you can try [hints](https://cmake.org/cmake/help/git-stage/module/FindPython3.html#hints) or [specifications](https://cmake.org/cmake/help/git-stage/module/FindPython3.html#artifacts-specification) to help CMake find the right version
4. Build the project, e.g. `make`
5. Open Blender and enable the mc_rtc addon in your preferences
6. The GUI can be activated by clicking the `mc_rtc GUI` button in the viewport gizmos menu

[BlenderImGui]: https://github.com/eliemichel/BlenderImgui
[mc\_rtc]: https://jrl-umi3218.github.io/mc_rtc/
[Blender]: https://www.blender.org/
[Python]: https://www.python.org/
