![Example](/images/vhacd-toolkit-0.gif)
![Example](/images/vhacd-toolkit-1.gif)

V-HACD [toolkit](https://www.youtube.com/watch?v=6Elao25HN9Y&list=PLWInthQ-GtLhzoyqhaJAvzico8mkXMyDI&index=1) on YouTube.  
V-HACD [Export](https://youtu.be/6Fh4-olKrs4) on YouTube.  
V-HACD use with [Unity](https://youtu.be/8CStATK1X5s) on YouTube.

**IMPORTANT:**
* Requires [hou-hdk-common](https://github.com/sebastianswann/hou-hdk-common) repository
* Read other requiments on mentioned above repository
* (Optional) Requires [JSON.Net for Unity](https://github.com/SaladLab/Json.Net.Unity3D) to make Unity scripts work properly.

**Currently supported platforms:**
- [x] Windows
- [x] MacOS
- [x] Linux

**TODO:**
- [ ] add export to FBX for UE4
- [ ] update HDAs
- [ ] update Python scripts
- [ ] port SOP Debug to C++
- [ ] port SOP Delete to C++
- [ ] port SOP Export to C++
- [ ] port SOP Generate to C++
- [x] port [SOP Merge](https://github.com/sebastianswann/hou-hdk-sop-vhacd-toolkit/tree/master/source/SOP_VHACDMerge) to C++
- [ ] port SOP Scout to C++
- [x] port [SOP Setup](https://github.com/sebastianswann/hou-hdk-sop-vhacd-toolkit/tree/master/source/SOP_VHACDSetup) to C++
- [ ] port SOP Transform to C++
- [x] rework [CMakeLists.txt](https://github.com/sebastianswann/hou-hdk-sop-vhacd-toolkit/blob/master/cmake/CMakeLists.txt) to automatically link [V-HACD](https://github.com/sebastianswann/hou-hdk-sop-vhacd-toolkit/tree/master/3rdParty/VHACD_Lib) library source files
