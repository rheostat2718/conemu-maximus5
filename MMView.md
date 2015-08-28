[Download (Far1/Far2 x86/x64)](http://code.google.com/p/conemu-maximus5/downloads/list?q=summary:MMView)

### About ###
‘Multimedia Viewer’ Far Manager plugin allows you to view pictures, video and listen audio without leaving FAR or launching any external programs.
Note, that it does not works in **real** fullscreen mode (this restriction is removed in ConEmu).

Plugin plays sounds and videos via Windows DirectShow. So, when file is not recognized, you must install required codecs.

Plugin works with pictures via [GFL graphic library (http://www.xnview.com)](http://www.xnview.com). Library might be named **libgfl311.dll**. You may choose GFL library location via registry
(look at _gfl.reg_ for example). If plugin by any reason can't use the library, it will show a short description
about the problem (it is possible to disable this warning). The current version of GFL library must be at least 3.11. Be carefull using the
newer versions of library: they may contain changes, making plugin working impossible.