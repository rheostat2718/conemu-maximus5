﻿#Summary Reporting ConEmu crashes

ConEmu is stable enough.

**Note!** I often have reports related to several MicrosoftBugs. If you see the
crash, please read [wiki](MicrosoftBugs.md) and make sure [Inject ConEmuHk](ConEmuHk.md)
feature is enabled!

Well, if any unknown crash was occured in `ConEmu` or `ConEmuC` processes,
MemoryDump may greatly help to locate and fix the problem.

Crashes in `ConEmu[64].exe` and `ConEmuC[64].exe` are handled automatically
and application suggests to user (message box) to create MemoryDump (full or mini).
Due to limitation of googlecode attachments, please upload dumps to DropBox
or any other hosting and post the link to issues or via email.

ConEmu does not handle crashes (exceptions actually) in the running console applications.
That is because to minimize intrusion to running application.

If you need to create dump manually (of any ConEmu's processes or console application)
please read that [wiki](MemoryDump.md).