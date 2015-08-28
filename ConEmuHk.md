﻿Injects? ConEmuHk.dll? All spawned processes in ConEmu consoles? Yes!

There are a lot of information exists in wiki already but let it be here, in one place.

ConEmu need to intercept large amount of Windows API functions, otherwise it is not possible
neither to avoid certain MicrosoftBugs nor implement much of juicy features.

One may said, other terminals works without injects perfectly... But that is not true.
For example, mintty is small, fast and works without injects, but you can't run there
many programs (PowerShell, Far Manager and many others using Windows console API).
Another well known terminal Console and its clones (it uses injects)  will have same problems
as ConEmu (for example, [Issue 643](https://code.google.com/p/conemu-maximus5/issues/detail?id=643) or [Issue 65](https://code.google.com/p/conemu-maximus5/issues/detail?id=65)).

## Tech info ##
What are injects? ConEmu provides two dlls ConEmuHk.dll and ConEmuHk64.dll which
will be loaded in each process of console processes started in ConEmu window.
Injections may be possible with SetThreadContext after CreateProcess.
When dll is loaded into target process it may hook Windows API (in that process only)
changing IAT pointers.

## Slowdown ##
Current method of injections and hooking will cause small lag when new process
is created in the ConEmu console. It is really small (about 60 ms per process on my virtual PC).
User may notice that lag in the only case of running hundreds and thousands of processes.

## Third party problems ##
I had bunches of issues about ‘crashes in ConEmu’. Really? And what are the causes?

  * MicrosoftBugs. Windows kernel has several bugs leading to crashes, hungs and black screens. Microsoft fix bugs in next windows versions only. Great, user needs to buy new Windows... Workaround? Injects only.
  * Third party tools run injects too. Several injection systems will conflict each other. The result? Crash! One system tries to execute LoadLibrary in the external process, but it think that LoadLibrary is located in the other module than kernel32.dll. Sadly.
    * MacType and sort of. There is workaround - add ConEmuC.exe and ConEmuC64.exe to MacType exclusion list. You may still use it with ConEmu.exe and ConEmu64.exe.
    * AnsiCon. Another ANSI sequences processor. ConEmu already implements that feature internally.
    * Intel pin. You may use it with wrapper batch file, disabling ConEmuHk temporarily. Read about [ConEmuHooks environment variable](ConEmuEnvironment.md).
  * ConEmu bugs. Hmm, show me completely bugless software? Report the problem and it will be fixed, I hope.

## Conclusion ##
Do you think still you have no need in ConEmuHk?
Well, you may **completely** disable them, but **don't complain** of arisen problems which are absent when injects are enabled.

  * Uncheck ‘Inject ConEmuHk’
  * Physically delete files ConEmuHk.dll and ConEmuHk64.dll.

At last, I have some plans about rewritting hooks code using different technique. But it will take time
and I can't predict what speed improvement can be achieved.