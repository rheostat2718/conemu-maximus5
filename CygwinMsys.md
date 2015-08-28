﻿#summary cygwin, msys, msysgit and ConEmu

# ConEmu has moved to conemu.github.io #
New versions of this file are available on [conemu.github.io](http://conemu.github.io/en/CygwinMsys.html).

# Abstract #
Report cygwin and msys bugs to their [authors](http://dir.gmane.org/gmane.os.cygwin)!

ConEmu is terminal!
It just the display for output of console applications.
Bugs in applications will lead wrong display output in ConEmu. Of course.

Still thinking the problem is inside ConEmu? Read the rest of this wiki.

# Some techinfo first #
There are two types of Windows console (terminal) emulators:

  1. Based on stdin/stdout redirection. Most known - [mintty](https://code.google.com/p/mintty/), [puttycyg](https://code.google.com/p/puttycyg/), [Poderosa's cygterm](https://sourceforge.net/projects/poderosa/).
  1. Based on Windows console API. Most known - ConEmu, [Console](https://sourceforge.net/projects/console/), [Take Command](http://jpsoft.com/).

# stdin/stdout redirection style #
Most of this type emulators are based on ‘POSIX layer’ -
[cygwin](http://cygwin.com/), [msys](http://www.mingw.org/wiki/MSYS) or [msysgit](http://msysgit.github.io/).
AFAIK, msys was forked long ago from cygwin, that is why they are alike in many cases.
In turn, there is another fork - msysgit was forked from msys...

## Pros ##
Very fast output. Really. No need to use ‘slow’ Windows console subsystem,
that's why the speed is limited mainly with pipe throughput.

‘Easy’ implementation of ANSI support. All ANSI codes are processed inside the emulator.
No need to translate them to and from Windows console subsystem.

Almost unlimited backscroll buffer.

## Cons ##
User can't run here large amount of programs designed for Windows console.
For example, Powershell and Far Manager can't be started at all.
**Official** Vim is not working.
cmd.exe can do something here, but command history (Up arrow) is not working.
And so on...

# Windows console API style #
These emulators always have hidden standard Windows console window.
Say, they are wrappers around Windows API.
That means all programs designed for Windows console sybsystem must be working perfectly.
Of course, using these emulators, users will get bonuses like easy resizing and handy copy/pasting.

On the other hand, programs designed for ‘POSIX layer’ may fails here or there...

## Complains ##
I regularly get emails and issues about cygwin/msys compatibility with ConEmu.
Most of them does not have anything to fix inside ConEmu sources,
but users complain me instead of cygwin/msys developers.

Let me draw an analogue. There is a great tool RPP, but it is an MacOS application. How can you run it on Windows? Set up VMWare with MacOS inside :-D

The same with POSIX console applications in Windows. If you want to run them properly - run them in mintty. That is weird for my taste...
If you develop Windows console application, it must be able to run in Windows standard console window.
Well, it may show some lack of features (like 256 colors processing) but it must be working properly in all other cases.

## Windows power ##
Windows standard console supports 16 color palette, 32K lines of backscroll buffer, alternative screens, Unicode, mouse input and dozens of other features.
The main complains of Linux users  - unfriendly clipboard support (copy/paste) and window resizing. Really, ‘unfriendly’ but not an ‘absent’.
So, if you are using ConEmu or similar emulator - copy/paste/resize is not a problem at all...

What I want to say. Windows console is powerful subsystem. Just run there proper shell. If you like bash - just run it!

# Problems with cygwin #
Before reporting cygwin-related problem to me, please check it first in the standard Windows console.
Just run your shell from `Win+R`, for example `C:\cygwin\bin\sh.exe -l -i`, ensure your console
window has the same buffer and window sizes (right click on the console caption, choose ‘Properties’
and change sizes), and try to reproduce your problem.

I believe, your problem will repeats in standard Windows console (many users erroneously call it ‘cmd.exe’).
At the moment, only one reported bug really relates to ConEmu: [Issue 1549](https://code.google.com/p/conemu-maximus5/issues/detail?id=1549) (clearing screen with `Ctrl+L`).

But in most of cases, your problem will repeats outside of ConEmu.
That means, you should contact cygwin team, because ‘console application must work properly in the console’.
But in your case, cygwin acts properly only under certain terminal emulators (mintty), which are not real consoles in fact.


# Report examples #

## Broken screen output ##
Was happen with alternative screens. There was several bugs related and they was fixed in cygwin.
As I said, nothing to do with ConEmu, just ask cygwin team to make it work in Windows console.
Seems like reported problems of that class are already fixed within [cygwin](http://cygwin.com/snapshots/).

Some related issues: [Issue 1397](https://code.google.com/p/conemu-maximus5/issues/detail?id=1397), [Issue 1537](https://code.google.com/p/conemu-maximus5/issues/detail?id=1537)

## Mouse do not working ##
Mouse do working in mintty but don't in standard Windows console.
For example, mc ignores mouse clicks when it is started from "cmd.exe".
That is because mintty send escape sequences to the terminal input (unix-like), but Windows standard describes special
input event - [MOUSE\_EVENT\_RECORD](http://msdn.microsoft.com/en-us/library/windows/desktop/ms684239(v=vs.85).aspx).
Take a look at Far Manager or **official** Vim. They perfectly support mouse input.

You may read more about console input in msdn:
[ReadConsoleInput](http://msdn.microsoft.com/en-us/library/windows/desktop/ms684961(v=vs.85).aspx),
[INPUT\_RECORD](http://msdn.microsoft.com/en-us/library/windows/desktop/ms683499(v=vs.85).aspx),
[MOUSE\_EVENT\_RECORD](http://msdn.microsoft.com/en-us/library/windows/desktop/ms684239(v=vs.85).aspx).

Some related issues: [Issue 943](https://code.google.com/p/conemu-maximus5/issues/detail?id=943), [Issue 1492](https://code.google.com/p/conemu-maximus5/issues/detail?id=1492), [Issue 1497](https://code.google.com/p/conemu-maximus5/issues/detail?id=1497), [gmane thread](http://thread.gmane.org/gmane.os.cygwin/146090/focus=146133).

## ANSI support ##
One may notice that colors in the mintty are "nice" but in the ConEmu are "ugly".
That is because cygwin (ncurses actually, thought) does not send escape sequences to the terminal output but process them internally.

Cygwin does not want to know anything about ConEmu. I can understand this point of view.

Read more in wiki CygwinAnsi.

## Startup directory ##
Cygwin shell always starts in your `"${HOME}"` instead of specified working folder.

How to change that behaviour - read in wiki CygwinStartDir.

# Resume #
My position is simple: if you are writing console application for Windows - just make it working in Windows.
Report your problems related to Windows console to cygwin team here: [dir.gmane.org/gmane.os.cygwin](http://dir.gmane.org/gmane.os.cygwin).