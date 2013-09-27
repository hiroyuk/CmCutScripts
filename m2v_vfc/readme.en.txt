What is this:
  VFAPI plugin for TMPGEnc/AviUtl

Supported format:
  MPEG-1 Video (Video ES / MPEG-1 SS / MPEG-2 PS / MPEG-2 TS)
  MPEG-2 Video (Video ES / MPEG-1 SS / MPEG-2 PS / MPEG-2 TS)
  MPEG-1 Audio Layer-2 (MPEG-1 SS / MPEG-2 PS)

Author:
  MOGI, Kazuhiro <kazhiro@marumo.ne.jp>

Download site:
  http://www.marumo.ne.jp/mpeg2/

License:
  The software is published for the test purpose. 
You may use the software under the following conditions

  a) You have to use the software frequently and check 
     the output video and audio carefully.
  b) You have to carry out a detailed report, when the
     software bug is discovered.
  c) The software is published with no warranty. 
     You may not claim an author's liability.
  d) You may not redistribute the archive without 
     modification.
  e) On your own liability, you can modify the source 
     code and freely use it.

How to install:
  1. put m2v.vfp and m2vconf.exe into the same folder.
  2. execute m2vconf.exe
  3. select options
  4. push OK button

About mme.exe:
  The mme.exe is a simple MPEG editor using m2v.vfp. 
It is able to save a part of MPEG file. 
  However, it is still in a pre-alpha version and many 
functions have not been implemented yet. 
  The following list is a non-implemented functions.

  + "Edit->Undo" menu (allways grayout)
  + "Help->About" menu (allways grayout)
  + saving TS file (allways failed)
  + triming audio (a bits longer/shorter than video)
  + etc...

If you want to add those functions, you shuld read
the C source code. (in the src\vfp\ and the src\mme\)

Bug report:
  mail:    bug@marumo.ne.jp
  sample:  ftp://www.marumo.ne.jp/incoming/

