![Example](/images/vhacd-toolkit-0.gif)
![Example](/images/vhacd-toolkit-1.gif)

V-HACD [toolkit](https://www.youtube.com/watch?v=6Elao25HN9Y&feature=youtu.be) on YouTube.  
V-HACD [Export](https://www.youtube.com/watch?v=6Fh4-olKrs4&feature=youtu.be) on YouTube.  
V-HACD use with [Unity](https://www.youtube.com/watch?v=8CStATK1X5s&feature=youtu.be) on YouTube.  
V-HACD updates [22-12-2017](https://www.youtube.com/watch?v=h2VCUtb8UfE&feature=youtu.be) on YouTube.

**IMPORTANT:**
* Requires [hou-hdk-common](https://github.com/sebastianswann/hou-hdk-common) repository
* Read other requiments on mentioned above repository
* (Optional) Requires [JSON.Net for Unity](https://github.com/SaladLab/Json.Net.Unity3D) to make Unity scripts work properly.

**Currently supported platforms:**
- [x] Windows
- [x] MacOS
- [x] Linux

**TODO:**
- [ ] add support for SOP Setup generated attributes in SOP Merge
- [ ] add export to FBX for UE4
- [ ] update HDAs
- [ ] update Python scripts
- [ ] port SOP Debug to C++
- [ ] port SOP Delete to C++
- [ ] port SOP Export to C++
- [x] port [SOP Generate](https://github.com/sebastianswann/hou-hdk-sop-vhacd-toolkit/tree/master/source/SOP_VHACDGenerate) to C++
- [x] port [SOP Merge](https://github.com/sebastianswann/hou-hdk-sop-vhacd-toolkit/tree/master/source/SOP_VHACDMerge) to C++
- [x] port [SOP Scout Jr.](https://github.com/sebastianswann/hou-hdk-sop-vhacd-toolkit/tree/master/source/SOP_VHACDScoutJunior) and [SOP Scout Sr.](https://github.com/sebastianswann/hou-hdk-sop-vhacd-toolkit/tree/master/source/SOP_VHACDScoutSenior) to C++
- [x] port [SOP Setup](https://github.com/sebastianswann/hou-hdk-sop-vhacd-toolkit/tree/master/source/SOP_VHACDSetup) to C++
- [ ] port SOP Transform to C++
- [x] rework [CMakeLists.txt](https://github.com/sebastianswann/hou-hdk-sop-vhacd-toolkit/blob/master/cmake/CMakeLists.txt) to automatically link [V-HACD](https://github.com/sebastianswann/hou-hdk-sop-vhacd-toolkit/tree/master/3rdParty/VHACD_Lib) library source files
