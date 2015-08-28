﻿#summary Change tcc prompt to insert mode by default

From time to time any user complains that ConEmu starts tcc (tcc/le) prompt in ‘Overwrite’ mode.
But that is not a ConEmu responsibility (check it with `cmd.exe` as a shell).
You need to configure your tcc properly.

Just run from yours tcc prompt:

```
OPTION
```

And choose desired mode on the ‘Command Line’ tab. Do not forget to press ‘OK’ button ;)

<img src='http://conemu-maximus5.googlecode.com/svn/files/TccInsertOverwrite.png' title='Change tcc prompt to insert mode by default'>

Actually, it will update your <code>TCMD.INI</code> file (located in <code>"%USERPROFILE%\AppData\Local\JPSoft"</code> or near to <code>tcc.exe</code>).<br>
<br>
<pre><code>[4NT]
CursorIns=15
CursorOver=100
EditMode=Insert
;; And so on...
</code></pre>

Alternatively, you may use the following command in your tcc prompt.<br>
But note that it will not update yours <code>TCC.INI</code> configuration file.<br>
<br>
<pre><code>SETDOS /M1 /S100:15
</code></pre>