When you start ConEmu and do not specify the directory when you want to start it
the default (startup) directory will be `%USERPROFILE%`.

Why? There are reasons.

  * It is almost useless to start your shell in the ConEmu installation folder (you will get that if you run ConEmu from shortcut or Explorer window).
  * It may be dangerous to start your shell in the system32 folder (you will get that if you run something from Win+R).

If you want to use another default directory there are options.

  * ConEmu's shortcut properties. You may change ‘Working directory’ field.
  * Task parameters. You may add `/dir` switch. This may be overrided by ConEmu's [command line](ConEmuArgs.md).
  * ConEmu's [command line](ConEmuArgs.md) (or shortcut). You may add `/dir` switch.
  * [-new\_console:d:...](NewConsole.md) switch in the task or specified command.