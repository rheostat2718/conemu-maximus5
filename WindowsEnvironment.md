With several exceptions defined in ConEmuEnvironment wiki, ConEmu inherent all variables from calling process (which is explorer.exe in most cases).

ConEmu does not change variables with exception of %PATH% (configurable on "ComSpec" settings page) and all %ConEmu...% variables.

**All** other variables are left intact!

That means, if you notice some strange in your environment - find problems on your side.

Some common problems described below.

## Variables doesn't match defined in system settings ##
Well, sometimes Windows Explorer fails with automatic acceptance of changes done by user or installation programs. This is known Microsoft behavior.

### Resolution ###
Logoff your session and logon again.

<br>

<h2>Something strange with home</h2>
Are you using clink? Are you %HOME% points to clink's profile? You may be using old clink version. There is an issue on clink's project site about that: <a href='https://code.google.com/p/clink/issues/detail?id=113&can=1&q=home'>https://code.google.com/p/clink/issues/detail?id=113&amp;can=1&amp;q=home</a>

<h3>Resolution</h3>
<ul><li>Update clink (if you are using it)<br>
</li><li>Use ProcessExplorer to examine environment variables of all parent processes (from your shell and above). That may give you a clue.