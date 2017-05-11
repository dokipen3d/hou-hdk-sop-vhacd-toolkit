# V-HACD toolkit for Houdini. #

![Example](/home/config/help/nodes/sop/vhacd-toolkit-0.gif)
![Example](/home/config/help/nodes/sop/vhacd-toolkit-1.gif)

Overview [video](https://www.youtube.com/watch?v=6Elao25HN9Y&list=PLWInthQ-GtLhzoyqhaJAvzico8mkXMyDI&index=1) on YouTube.

**IMPORTANT:**
* Requires [hou-hdk-common](https://github.com/sebastianswann/hou-hdk-common) repository
* Read other requiments on mentioned above repository

**Currently supported platforms:**
- [x] Windows
- [ ] Apple
- [ ] Linux

**TODO:**
- [ ] update HDAs
- [ ] update Python scripts
- [ ] rework CMakeLists.txt to automatically link V-HACD library source files

## LEGEND (common for all plugins):
* [source](/source) folder contains `.h` and `.cpp` files required to compile DSO.
* [cmake](/cmake) folder contains additional modules used by `CMakeLists.txt`.
* `build` folder will be automatically created by CMake process and will contain Visual Studio project files.
* `home/dso` folder will be automatically created by compilation process and will contain compiled plugin.
* *(optional)* `3rdParty` folder contains additional `.h`, `.cpp`, `.dll` and `.lib` files, provided by 3rd parties.
* *(optional)* `home/otls` folder contains HDAs that accompany compiled DSO.
* *(optional)* `home/icons` folder contains icons that are used by compiled DSO and/or HDAs.
* *(optional)* `home/scripts` folder contains contains python and VEX scripts.