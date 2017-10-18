# V-HACD toolkit for Houdini. #

![Example](/images/vhacd-toolkit-0.gif)
![Example](/images/vhacd-toolkit-1.gif)

V-HACD [toolkit](https://www.youtube.com/watch?v=6Elao25HN9Y&list=PLWInthQ-GtLhzoyqhaJAvzico8mkXMyDI&index=1) on YouTube.
V-HACD [Export](https://youtu.be/6Fh4-olKrs4) on YouTube.
Example of V-HACD use with [Unity](https://youtu.be/8CStATK1X5s) on YouTube.

**IMPORTANT:**
* Requires [hou-hdk-common](https://github.com/sebastianswann/hou-hdk-common) repository
* Read other requiments on mentioned above repository
* (Optional) Requires [JSON.Net for Unity](https://github.com/SaladLab/Json.Net.Unity3D) to make Unity scripts work properly.

**Currently supported platforms:**
- [x] Windows
- [x] MacOS
- [x] Linux

**TODO:**
- [ ] update HDAs
- [ ] update Python scripts
- [ ] port SOP Debug to C++
- [ ] port SOP Delete to C++
- [ ] port SOP Export to C++
- [ ] port SOP Generate to C++
- [ ] port SOP Merge to C++
- [ ] port SOP Scout to C++
- [x] port [SOP Setup](/source/SOP_VHACDSetup) to C++
- [ ] port SOP Transform to C++
- [x] rework [CMakeLists.txt](/cmake/CMakeLists.txt) to automatically link [V-HACD](/3rdParty/VHACD_Lib) library source files