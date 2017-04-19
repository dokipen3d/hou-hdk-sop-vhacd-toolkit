**IMPORTANT:**
* Requires [hou-hdk-common](https://github.com/sebastianswann/hou-hdk-common) repository
* Read other requiments on mentioned above repository

**TODO:**
- [ ] update HDAs
- [ ] update Python scripts
- [ ] rework CMakeLists.txt to automatically link V-HACD library source files

## LEGEND (common for all plugins):
* [source](/source) folder contains `.h` and `.cpp` files required to compile DSO.
* `build` folder will be automatically created by CMake process and will contain Visual Studio project files.
* `home/dso` folder will be automatically created by compilation process and will contain compiled plugin.
* *(optional)* `home/otls` folder contains HDAs that accompany compiled DSO.
* *(optional)* `home/icons` folder contains icons that are used by compiled DSO and/or HDAs.
* *(optional)* `home/scripts` folder contains contains python and VEX scripts.