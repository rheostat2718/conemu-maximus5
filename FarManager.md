#labels Featured
Page still under development. If you can help with writing, have questions or suggestions - leave comments here :)

<font size='5'><b>Far Manager</b></font>

Far is my favorite shell. It is not only file/archive management,
it is powerful editor, huge number of plugins, powerful macro engine.



# First of all #

## There are two platforms (?) ##
  * x86 (32-bit binaries) it works everywhere, beginning from Windows 2000 SP4 with one hotfix.
  * x64 (64-bit binaries) it requires 64-bit OS.

And yes, plugins compiled for x86 does not works in x64 Far Manager and vice versa. You owe to use appropriate plugin versions.

Most developers creates binaries with obvious names.

This is the general mistake of new users. They install x64 Far Manager and try to use x86 versions of plugins.
So, they asks ‘Why plugin does not appears in the list?’

IMHO, there is no considerable difference between them, but not every plugin has x64 version.

I'm using Far 3.0 x86 version (my primary OS is Win 7 x64).

## There are three (already) branches of Far Manager ##
  * 1.x (closed) oldschool version. It is ANSI (OEM to be more precise),
so it can't deal with unicode characters out of system OEM codetable.
However, some users believe ‘the best the enemy of good’ and do not switch to newer versions.
All settings are stored in registry `HKEY_CURRENT_USER\Software\Far`.
  * 2.x (closed). General difference - unicode support (file system, editor, viewer).
There was API break, but 2.x natively supports most of 1.x plugins via internal plugin wrapper.
All settings are stored in registry `HKEY_CURRENT_USER\Software\Far2`.
  * 3.x (curren). General difference - Far Manager settings and native (3.x) plugins settings are stored in SQLiteDB.
API was completely redesigned (and is not stable for current day). Many bugfixes and improvements.
Far 3.x can still deal with ANSI plugins (from Far 1.x),
but due to API break can't work with ‘old’ unicode plugins (from Far 2.x).
However! There is [external plugin wrapper](http://code.google.com/p/conemu-maximus5/wiki/Far3Wrapper).
Through it you can use many plugins from Far 2.x.

## There are several types of plugins ##
  * **Panels**. This is most common type. Examples:
    * Miscellaneous archives: Arclite, [Observer](http://forum.farmanager.com/viewtopic.php?f=11&t=4643), [MultiArc](http://www.farmanager.com/files/Far175b2634.x86.20110203.7z) (from Far 1.7x) - these are the basic
    * Remote access: Network, Pocket and Phone, [Shell extension](http://forum.farmanager.com/viewtopic.php?p=81676#p81676), [SFTP/SCP/FTP/FTPS/WebDAV](http://forum.farmanager.com/viewtopic.php?f=11&t=6316), [Registry](http://code.google.com/p/conemu-maximus5/wiki/RegEditor) and so on
  * **C0 panel plugins**. These plugins works in panels, but does not emulates virtual file system. They provide data for special ‘С0’ column for real file system:
    * ver\_c0 - show versions for PE files (exe, dll, sys, ocx, ...)
    * svn\_c0 - revision and status, only svn 1.6.x
    * pdf\_c0, html\_c0 - show title of document
    * mp3\_c0 - artist/title for MP3 files (ID1)
    * diz\_c0 - descript.ion, files.bbs, etc.
  * **Modal**. In most cases these plugins executes some specific action. Examples:
    * Uninstall - display list of installed applications and allows Modify/Repair/Uninstall. This is the same as Windows ‘Uninstall or change a program’ but much faster and support fast filtering.
    * HexEditors - as is :)
    * Bulk file renamers
    * Picture viewers, Audio/Video players, yes, it is possible in console :)
    * Unicode CharMap
  * **Editor related**
    * Searching, sorting, filtering, line wrapping, syntax hilighting...
  * **Non categorized**. These plugins can't be placed in specific category, for example
    * Regular Expression Search - it work in Editor and panels


# 1. Downloads / Installation #
  1. Download Full version from [official site](http://www.farmanager.com/download.php?l=en). Does not matter whatever you use - msi installer or 7z package.
  1. (Optionally) update binary files with my [Far3bis](http://sourceforge.net/projects/conemu/files/FarManager/Far3bis). List of improvements [here](http://github.com/Maximus5/Far3bis/wiki/ListOfPatches). In breaf - improvements of plugin manager (AltShiftF9), optimization of C0 column.

After installing (unpacking) Far Manager you have such directory structure (by example of Far 3.x)
|.\Far.exe<br>.\FarEng.hlf<br>.\FarEng.lng<table><thead><th>General Far Manager files - executable, help, language data</th></thead><tbody>
<tr><td>.\Addons                                 </td><td>Sample color schemes, tweaks, macros                       </td></tr>
<tr><td>.\Documentation\eng                      </td><td>Some useful information, on english :)                     </td></tr>
<tr><td>.\Plugins                                </td><td>Here, in subfolders, plugins are located                   </td></tr></tbody></table>

<h1>2. Install plugins to your's mind</h1>
There thousands of plugins written for many years. There many sources, where you can download them.<br>
<br>
Installing choosen plugin (in most cases) means create subfolder in ‘Plugins’ and unpack plugin distribution to this folder.<br>
<br>
To make Far Manager load new plugins you may:<br>
<ul><li>Simply restart Far Manager<br>
</li><li>or, for <a href='https://sourceforge.net/projects/conemu/files/FarManager/'>bis</a> versions, press AltShiftF9 in panels to open ‘Plugins configuration’ menu, and press CtrlR<br>
</li><li>or, navigate to plugin folder, type in command prompt ‘load:plugin_file_name.dll’ and press Enter.</li></ul>

<h2>Examples</h2>
<ol><li>(Note, this plugin already included in the latest Far distro). You have downloaded <a href='https://sourceforge.net/projects/colorer/files/FAR%20Colorer/'>FarColorer_far3_1.0.3.10.7z</a> (syntax hilighting in editor). This archive already contans root folder ‘FarColorer’. So, just unpack 7z package to ‘Plugins’ and restart far. Note, this 7z package contains both x86 and x64 versions of FarColorer.<br>
</li><li>You have downloaded <a href='http://code.google.com/p/conemu-maximus5/wiki/RegEditor'>RegEdit3.1.1.36.2580.7z</a>. This package does not have root folder, so create and unpack package files to ‘Plugins\RegEdit’. Note, this 7z package contains both x86 and x64 versions of plugin.</li></ol>

<h2>Main plugins sources</h2>
<ul><li><a href='http://plugring.farmanager.com/index.php?l=en'>PlugRing</a>
</li><li><a href='http://forum.farmanager.com/viewforum.php?f=11'>Plugin announces (russian)</a>
</li><li><a href='http://forum.farmanager.com/viewtopic.php?f=5&t=4590'>Plugin dashboard 1</a>, <a href='http://forum.farmanager.com/viewtopic.php?f=5&t=3478'>dashboard 2</a>.</li></ul>

<h2>Developer sites</h2>
<ul><li><a href='https://github.com/Maximus5/FarPlugins'>Regeditor, Picture View, ImpEx (PE browser), Far3wrap and others</a> and <a href='http://code.google.com/p/conemu-maximus5/wiki/FarPlugins'>old mirror</a>.<br>
</li><li><a href='http://code.google.com/p/far-plugins/'>MacroLib, Unicode CharMap, Visual Compare, and others</a>
</li><li><a href='http://code.google.com/p/far-macro-library/wiki/Common?wl=en'>Macros repository</a>
</li><li><a href='http://sourceforge.net/projects/far-observer/'>Observer multi-format (msi/pst/iso/mime/pdb) browser</a>
</li><li><a href='http://code.google.com/p/farplug-alvls/'>farplug-alvls</a>
</li><li><a href='http://code.google.com/p/farplugs/'>Hexitor (Hex editor), Media Info, Image Info, SQLiteDB, Disk menu and others</a>
</li><li>(Note, this plugin already included in the latest Far distro). <a href='http://colorer.sourceforge.net/farplugin.html'>Colorer (syntax hilighting in Editor, even in truecolor)</a>
</li><li><a href='https://code.google.com/p/evil-programmers/'>Many plugin sources</a></li></ul>

<h1>3. Help, FAQ, Documentation</h1>
Unfortunately for non-Russian speaking users, most of resources are on russian.<br>
However, you may use google translate, or ask competent users, for example, on<br>
<a href='http://forum.farmanager.com/index.php'>official forum</a>. Or, may be on<br>
<a href='http://superuser.com/questions/tagged/far-manager'>SuperUser</a>? :)<br>
<br>
<ul><li><a href='http://forum.farmanager.com/viewtopic.php?t=5207'>Far 3.0 FAQ (russian)</a>
</li><li><a href='http://www.farmanager.com/opensource.php?l=en'>Links for plugin developers</a></li></ul>

<h1>4. Control</h1>
Far have oldschool and intuitive NC-like interface :)<br>
<br>
Look on keybar press modifiers (Alt/Ctrl/Shift) and you'll see function keys actions. Most useful are (in panels)<br>
<table><thead><th>F1</th><th>Yes! Read it first. BTW, FarEng.hlf is a simple text file, you can open it in any editor.</th></thead><tbody>
<tr><td>ShiftF1</td><td>Create new archive from selected files                                                   </td></tr>
<tr><td>ShiftF2</td><td>Extract files from selected archive                                                      </td></tr>
<tr><td>CtrlPgDn</td><td>In most cases it is like as Enter, but generally, CtrlPgDn forces enter into archive, but it 'execution'. Compare behavior for <code>*.exe</code> files</td></tr>
<tr><td>F2</td><td>Configurable multilevel user menu                                                        </td></tr>
<tr><td>F3, F4</td><td>File viwer and editor                                                                    </td></tr>
<tr><td>F5, F6</td><td>Rename or move. Use ShiftF5 or ShiftF6 to copy/rename single file/folder under cursor    </td></tr>
<tr><td>AltF6</td><td>Create hard or symbolic link. Also, F7 can create links to any folder                    </td></tr>
<tr><td>F7</td><td>Create folder                                                                            </td></tr>
<tr><td>F8</td><td>Delete file/folder (to recycle, by default)                                              </td></tr>
<tr><td>ShiftDel</td><td>Delete file/folder pass by recycle                                                       </td></tr>
<tr><td>AltDel</td><td>Wipe files/folder                                                                        </td></tr>
<tr><td>AltF8</td><td>Show history of executed commands                                                        </td></tr>
<tr><td>F9</td><td>Pull down menu                                                                           </td></tr>
<tr><td>F10</td><td>Quit                                                                                     </td></tr>
<tr><td>F11</td><td>Open list of plugins, available for current context                                      </td></tr>
<tr><td>AltF11</td><td>File viewer/editor history                                                               </td></tr>
<tr><td>F12</td><td>Show screens (Panel/Editors/Viewers)                                                     </td></tr>
<tr><td>CtrlG</td><td>Group operations on files\folders                                                        </td></tr>
<tr><td>Ins, Gray+, Gray-, Gray</td><td>Selecting/deselecting files                                                              </td></tr></tbody></table>

This is <b>very</b> breaf list of key kombos, read F1 for more information.<br>
<br>
<h1>5. Automation</h1>
One of most powerful options of Far Manager are Macros. There are two styles of Macro<br>
<ul><li>Sequence of recorded keypresses. This is very useful, when you need to repeat multiple times one routine operation, or for simplifying access to most useful operations.<br>
<ul><li>For example, how to create oldschool keymapping - Esc turns panels on/off.<br>
<ul><li>Press Ctrl+. (Ctrl and Dot on main keyboard) to start recording (red ‘R’ lights in upper-left corner)<br>
</li><li>Press keys, the sequence: Ctrl+O<br>
</li><li>Press Ctrl+. again, recording stopped, Far show small dialog ‘Press the desired key’.<br>
<ul><li>Some ‘keys’ may be selected only from drop-down list. Choose ‘Esc’ from there.<br>
</li></ul></li><li>Press ‘Enter’ to confirm key selection.<br>
</li></ul></li><li>If you want to use recored macro in future sessions, you owe to save it<br>
<ul><li>Press ShiftF9 in panels and confirm saving<br>
</li><li>or, type in command line ‘macro:save’ and press ‘Enter’.<br>
</li></ul></li><li>If you do not need recorded macro anymore<br>
<ul><li>Press (twice) Ctrl+. Ctrl+.<br>
</li><li>Choose macro key<br>
</li><li>Press ‘Enter’, confirm deletion<br>
</li></ul></li></ul></li><li>Scripts, written on <a href='http://translate.google.ru/translate?hl=ru&sl=ru&tl=en&u=http%3A%2F%2Fapi.farmanager.com%2Fru2%2Fmacro%2Fmacrocmd%2Findex.html'>Macro language</a>. There are several ways to create such macros<br>
<ul><li>Press ‘Ctrl+.’ any keys (‘Space’ for example) ‘Ctrl+Shift+.’ choose key ‘Enter’. Far opens dialog with recorded key sequence. You can modify it there.<br>
</li><li>Use <a href='http://translate.google.ru/translate?hl=ru&sl=ru&tl=en&u=http%3A%2F%2Fcode.google.com%2Fp%2Ffar-plugins%2Fwiki%2FMacroLib'>MacroLib</a>. This plugin (my choice) simplify creation, editing, maintaning and calling Far Macros. Read plugin documentation for description of fml (text) file format.<br>
</li><li>Use <code>*.farconfig</code> files. Generally, these files may contain any Far Manager settings, not Macros only. Import them with command <code>far.exe /import &lt;farconfig-file&gt;</code>.