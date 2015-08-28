Running simple ChildGui is a "side-feature".
ConEmu can't control or customize those apps.
In most cases console versions are preferred because
they are running in the ConEmu "native" mode.

<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuChildGui.png' title='ConEmu with two ChildGui'>

ChildGui is a term defines GUI applications which are drawing<br>
their contents itself using WinApi. Few examples: PuTTY, mintty,<br>
notepad, GViM and so on. Don't confuse them with ConsoleApplication's<br>
which are specially compiled. ChildGui, even they have their<br>
own console-like interfaces are not ConsoleApplication's.<br>
<br>
<b>Note</b> To be able to ‘integrate’ ChildGui into ConEmu,<br>
your application window must be <b>resizeable</b>! For example,<br>
if your PuTTY settings locks its window size to certain<br>
‘rows x cols’ values, its window is non-resizeable, and<br>
ConEmu will not integrate it into ConEmu's tabs.<br>
Because it is not ‘possible’ to resize that ChildGui<br>
when ConEmu window is to be resizing!<br>
<br>
One more note about <b>hotkeys</b>. When you are running ChildGui,<br>
all keypressed must be passed to that application.<br>
That's why most of ConEmu hotkeys will be inaccessible.<br>
With exception of hotkeys types ‘Global’, ‘Local’ and<br>
hotkeys with ‘Win’ modifier, if option ‘Install keyboard hooks’<br>
is <b>enabled</b>.<br>
<br>
At last. How to run ChildGui in ConEmu? Absolutely the same<br>
way you do that for simple ConsoleApplication's. For example,<br>
from ‘Create new console dialog’.<br>
<br>
<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuChildGui2.png' title='Running ChildGui in ConEmu'>