#summary Current vs Startup directory



# About directories #

When you run new console the shell is starting in the directory you specified.
That is the ‘Startup directory’.

<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuCreate.png' alt='ConEmu new console' title='ConEmu confirmation of new console creation'>

When you work with shell and do some CD's or run your scripts,<br>
its ‘Current directory’ may be (and will) changed. But directory<br>
is changed inside your <a href='TerminalVsShell.md'>Shell</a> but not inside terminal.<br>
ConEmu does not handle your shell commands when your execute them.<br>
Moreover, some shells (PowerShell, bash, etc.) doesn't even call<br>
Window API function ‘SetCurrentDirectory’, they handle their<br>
working directories internally.<br>
<br>
However, since ConEmu's build 140818 you get full support of ‘Current directory’ (<b>CD</b>).<br>
<br>
<ul><li>Automatic detection of <b>CD</b> in <code>cmd.exe</code> or <code>tcc.exe</code>;<br>
</li><li>Ability to inform ConEmu GUI about shell <b>CD</b> using <a href='http://conemu.github.io/en/AnsiEscapeCodes.html'>ANSI</a> or <a href='http://conemu.github.io/en/ConEmuC.html'>ConEmuC</a>;<br>
</li><li>Display <b>CD</b> in the tab title using <a href='http://conemu.github.io/en/SettingsTabBar.html'>tab templates</a>;<br>
</li><li>Reuse <b>CD</b> in the <a href='RestartTab.md'>Restart</a> and <a href='http://conemu.github.io/en/LaunchNewTab.html'>Create new console</a> dialogs;<br>
</li><li>At last you may use "<b>%CD%</b>" variable with <a href='http://conemu.github.io/en/GuiMacro.html#List_of_functions'>Shell</a> macro function in the <code>Dir</code> parameter.</li></ul>

<h1>What you shall do to get <b>CD</b> support</h1>

<h2>cmd and tcc</h2>
Just enable ‘<a href='http://conemu.github.io/en/ConEmuHk.html'>Inject ConEmuHk</a>’ feature. ConEmu will maintain <b>CD</b> for you automatically.<br>
<br>
<h2>bash and other cygwin shells</h2>
You need to tell bash to run <code>"ConEmuC -StoreCWD"</code> command each time its prompt executed.<br>
For example, add to your <code>.bashrc</code>
<pre><code>PROMPT_COMMAND='ConEmuC -StoreCWD'<br>
</code></pre>

<h2>PowerShell</h2>
You need to modify your profile to override prompt function. Just run in your PowerShell prompt:<br>
<pre><code>notepad $Profile<br>
</code></pre>
And change prompt as in the following example:<br>
<pre><code>function prompt {<br>
  # Just prettify the prompt<br>
  Write-Host -NoNewline -ForegroundColor Cyan "PS "<br>
  $dir = $(get-location).ProviderPath<br>
  Write-Host -NoNewline -ForegroundColor Yellow $dir<br>
  # You may use ANSI or direct ConEmuC call<br>
  # Write-Host -NoNewline (([char]27) + "]9;9;`"" + $dir + "`"" + ([char]27) + "\")<br>
  &amp; ConEmuC -StoreCWD "$dir"<br>
  return "&gt;"<br>
}<br>
</code></pre>