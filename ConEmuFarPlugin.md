Several Far Manager plugins works in the ConEmu only,
others provides advanced features in the ConEmu:
  * [ConEmu](ConEmuFarPlugin#ConEmu.md) (main connection between Far and ConEmu)
  * [ConEmu Panel Views](ConEmuFarPlugin#ConEmu_Panel_Views.md)
  * [ConEmu Underlines](ConEmuFarPlugin#ConEmu_Underlines.md)
  * [ConEmu Background](ConEmuFarPlugin#ConEmu_Background.md)
  * [PanelColorer](ConEmuFarPlugin#PanelColorer.md)
  * [FarColorer](ConEmuFarPlugin#FarColorer.md)

### ConEmu ###
ConEmu.dll (or ConEmu.x64.dll) provide next functionality:
  * Shell style Drag-and-Drop;
  * Tabs and window switching;
  * Some plugins depends on on ([FarHints](http://code.google.com/p/far-plugins/wiki/FarHints), [PictuewView](http://code.google.com/p/conemu-maximus5/wiki/PicView), [Multimedia Viewer](http://code.google.com/p/conemu-maximus5/wiki/MMView), etc.);
  * It shows real console, on abnormal ConEmu.exe termination;
  * It attach new real console, which appears after detach (Ctrl-Alt-Tab);
  * Shows long console output;
  * and so on.

ConEmu plugin itself have no configuration dialog. All its features are available
via Far Manager plugin menu (F11), You'll see it as «ConEmu».

**ConEmu plugin menu** <br>
<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuFarPlugin.png' alt='ConEmu FAR plugin' title='ConEmu FAR plugin'>

«Attach to ConEmu» is available only in «clear» console, when You starts FAR without ConEmu.<br>
<br>
<h3>ConEmu Panel Views</h3>
Enables thumbnails and tiles in Far Manager panels.<br>
<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuPanelViews.png' title='ConEmu Thumbnails and Tiles'>

Recommended way for Panel Views activation is Far Manager macro (look at <code>Thumbnails.reg</code> in ConEmu folder).<br>
<br>
Another way (direct) is Far Manager plugin menu (F11) («ConEmu Panel Views» item):<br>
<br>
<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuFarTh.png' title='ConEmu Panel Views plugin'>.<br>
<br>
You may polish Panel Views appearence via ConEmu <a href='Settings#Views.md'>Settings</a>.<br>
<br>
<h3>ConEmu Underlines</h3>
Simple background plugin:<br>
<br>
<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuFarLn.png' title='ConEmu Underlines'>.<br>
<br>
<h3>ConEmu Background</h3>
This plugin colorize Far panels, display mnemonic picture (drive, network, and so on), and progress bar<br>
of used drive space at status line area. ConEmu Background can be customized via Background.xml configuration file.<br>
This is «native» analog of <a href='ConEmuFarPlugin#PanelColorer.md'>PanelColorer</a> plugin.<br>
<br>
<img src='http://conemu-maximus5.googlecode.com/svn/files/PanelColorer.png' title='ConEmu Background'>.<br>
<h3>PanelColorer</h3>
This «third-party» background plugin is originally from <a href='http://forum.farmanager.com/viewtopic.php?f=11&t=5702'>here</a>.<br>
<a href='Hidden comment: 
You may download latest binary [http://filekeeper.org/download/browser.php?path=conemu/PanelColorer/ here]
'></a><br>
<br>
<h3>FarColorer</h3>
This <a href='http://colorer.sourceforge.net/farplugin.html'>plugin</a> works in plain Far Manager too,<br>
but when You choose <a href='http://www.farmanager.com/download.php'>Far 3.x</a>
or <a href='http://forum.farmanager.com/viewtopic.php?p=1041#p1041'>Far TrueMod 2.x</a>
and run it under ConEmu - You got full true colors in the console (Far editor only in Far TrueMod 2.x)<br>
instead of standard 16 (or 32 with <a href='Settings#Colors.md'>Extend foreground colors</a> option) console colors.<br>
<br>
<b>Note</b>, You must enable <a href='Settings#Colors.md'>Colorer TrueMod support</a> option.<br>
<br>
<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuTrueMod.png' title='ConEmu & FarColorer TrueMod'>.