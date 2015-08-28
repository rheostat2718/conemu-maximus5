==How to enable 256-color console Vim syntax highlight in ConEmu==

<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuVimXterm.png' title='256-colors vim in ConEmu'>

<a href='http://stackoverflow.com/a/14434531/1405560'>StackOverflow answer</a>

<ul><li>Vim's executable must be named ‘vim.exe’;<br>
</li><li>Check options ‘Inject ConEmuHk’ and ‘ANSI X3.64 / xterm 256 colors’ on <a href='SettingsFeatures.md'>Features</a> page;<br>
</li><li>Check option ‘TrueMod (24bit color) support’ on <a href='SettingsColors.md'>Colors</a> page;<br>
</li><li>Edit your ‘vimrc’ file, sample lines are here. Of course, you need some 256-color vim scheme, it is ‘zenburn’ in the last line of this example.</li></ul>

<pre><code>if !has("gui_running")<br>
    set term=xterm<br>
    set t_Co=256<br>
    let &amp;t_AB="\e[48;5;%dm"<br>
    let &amp;t_AF="\e[38;5;%dm"<br>
    colorscheme zenburn<br>
endif<br>
</code></pre>

You may also check environment variable <a href='ConEmuEnvironment.md'>ConEmuANSI</a> but I'm not sure how to do that properly.<br>
<br>
<b>Note</b>. GIT's Vim and ‘Original’ Vim (from gvim73_46.exe) are passed tests. MinGW's Vim fails to switch to using Ansi sequences.<br>
<br>
<br>
<h2>How to enable Vim scrolling using mouse Wheel in ConEmu</h2>

When ConEmu emulates xterm it translates mouse wheel to the following sequences:<br>
<br>
<table><thead><th> <b>Event</b> </th><th> <b>Sequence</b> </th><th> <b>Description</b> </th></thead><tbody>
<tr><td> <code>&lt;WheelDown&gt;</code> </td><td> "<code>\e[62~</code>" </td><td> toward the user    </td></tr>
<tr><td> <code>&lt;WheelUp&gt;</code> </td><td> "<code>\e[63~</code>" </td><td> away from the user </td></tr>
<tr><td> <code>&lt;Shift&gt;&lt;WheelDown&gt;</code> </td><td> "<code>\e[64~</code>" </td><td> toward the user    </td></tr>
<tr><td> <code>&lt;Shift&gt;&lt;WheelUp&gt;</code> </td><td> "<code>\e[65~</code>" </td><td> away from the user </td></tr></tbody></table>

So all you need to add following lines to your <code>vimrc</code> file:<br>
<br>
<pre><code>""""""""""""""""""""""""""""""""""""""<br>
" let mouse wheel scroll file contents<br>
""""""""""""""""""""""""""""""""""""""<br>
if !has("gui_running")<br>
    set term=xterm<br>
    set mouse=a<br>
    set nocompatible<br>
    inoremap &lt;Esc&gt;[62~ &lt;C-X&gt;&lt;C-E&gt;<br>
    inoremap &lt;Esc&gt;[63~ &lt;C-X&gt;&lt;C-Y&gt;<br>
    nnoremap &lt;Esc&gt;[62~ &lt;C-E&gt;<br>
    nnoremap &lt;Esc&gt;[63~ &lt;C-Y&gt;<br>
endif<br>
</code></pre>