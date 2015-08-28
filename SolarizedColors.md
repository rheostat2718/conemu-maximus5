#summary Solarized, git and settings

[Solarized](http://ethanschoonover.com/solarized) is an eye-candy color scheme specially designed
for console and GUI applications.

<img src='http://conemu-maximus5.googlecode.com/svn/files/SolarizedGit.png' title='ConEmu: Solarized Git'>

But this scheme has eight monotones and eight accent colors in opposite<br>
to the Windows standard palette and many others which has twelve accent colors.<br>
<br>
That may be a problem for some console applications which suppose that both<br>
lower and upper color octets has accented colors. Look at screenshot,<br>
colors 0..15 show Solarized palette, colors 16..31 show standard Windows colors.<br>
<br>
<img src='http://conemu-maximus5.googlecode.com/svn/files/Settings-Colors.png' title="ConEmu: Settings' Colors">

If your application looks weird, some colors are fade or undistinguished there are two ways.<br>
<br>
<h2>Setup your application</h2>
For example, <a href='http://msysgit.github.io/'>MsysGit</a> uses low color octet (dark green actually)<br>
for displaying branch names, updated (indexed) files, new diffs' lines and may be smth else.<br>
Just tell git use ‘<code>green bold</code>’ insted.<br>
<br>
<pre><code>git config --global color.diff.new "green bold"<br>
git config --global color.status.updated "green bold"<br>
git config --global color.branch.current "green bold"<br>
</code></pre>

<h2>Use modified color scheme</h2>
ConEmu has several Solarized schemes. For example <code>&lt;Solarized Git&gt;</code> was optimized<br>
for vanilla <a href='http://msysgit.github.io/'>MsysGit</a>, <code>&lt;Solarized (Luke Maciak)&gt;</code> has<br>
accented colors in lower octet, and so on. Just choose the desired scheme from<br>
dropdown list box.<br>
<br>
<img src='http://conemu-maximus5.googlecode.com/svn/files/Settings-Colors3.png' title="ConEmu: Settings' Colors">