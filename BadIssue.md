﻿#summary Why bad issues has no sense

# Be verbose #
Appreciating yours reporting efforts, but **only properly written issues are meaningful**.

Bad issues has no sense, developers can't fix them and reporters waste their time (both reporters and developers).

For example,

  * ‘There was some error, but I will not tell you what’
  * ‘I was doing something, do not remember what, and application crashes’
  * ‘Crash’

Wondering, why some users do not even read what they posting?
If reporter want to help developer make the application better,
or reporter want some fix in the application, he must provide
enough information to reproduce or locate the problem.

Software has many-many lines of code and it is impossible
to fix something abstract...

## Software version ##
One of the most significant information, omitted by many reporters.
And do not say ‘I'm using last version’. Only numbers!

ConEmu version is visible in the status bar, Settings or About dialogs.
Just press `Win+Alt+A` to be sure (SystemMenu \ Help \ About).

### Update your installation ###
If you are using old build, there is big chance that your problem was
fixed already. Why not to update?

Note, if you are using third-party bundles (like cmder or chocolatey)
you may not using latest build. There is ConEmu **internal** updater.
Just call it from SystemMenu \ Help \ Check for updates.
Or visit download page at [FossHub](http://www.fosshub.com/ConEmu.html).

Also, thanks to googlecode for stopping Download service, if you are using
**very old build** (pre 140115) can be **automatically** updated in **two steps** only.

## OS version ##
The Windows version matter! From version to version Windows behavior differs.
And there are serious MicrosoftBugs, wich may be fixed my Microsoft only.
Fortunaterly, in the most cases developer can create some workaround.

## Screenshot ##
Personally, I prefer [ShareX](http://getsharex.com/) to make screenshots,
but you may use any other program. Even pressing `PrintScreen`, running
`MSPaint` from `Win+R` and pressing `Ctrl+V`.

Fullsized screenshot may tell about the problem more than you can imagine.
Reporter may omit something from the problem description, but screenshot
will not omit anything.

Do not cut them and status line visibility is strongly recommended.

## Text of the error ##
If the error message box appears, screenshot may helps, of course.
But the text itself will be helpful. And reporter do not need to
re-type the text! Just press `Ctrl+C` in the dialog box, and its
message will be placed to the Windows clipboard. Tada...
Just paste it to the issue text.

## Crash dumps ##
When crash or assertion occures, created [Memory Dump](MemoryDump.md)
will be very helpful! In most cases, Memory Dump may tell developer
the exact location and conditions of the problem.

Due to large size of created dumps, do not upload/attach them
to the issues. Upload them to DropBox or any other hosting and
post the link (via email if you do worry about availability to public).