﻿#summary The problem with apphelp.dll

<a href='Hidden comment: 
https://bugs.php.net/bug.php?id=36055
https://bugzilla.mozilla.org/show_bug.cgi?id=939043
'></a>

## Abstract ##
Several times I've seen cygwin/msys errors when their subsystem had failed to create child processes.

### Example from cygwin's ls ###
```
0 [main] us 0 init_cheap: VirtualAlloc pointer is null, Win32 error 487 AllocationBase 0x68520000, BaseAddress 0x68570000, RegionSize 0x218000, State 0x1000 C:\Program Files (x86)\Git\bin\ls.exe: *** Couldn't reserve space for cygwin's heap, Win32 error 487
```

### Example from msys-git's perl ###
```
С:\GIT\bin\perl.exe: *** unable to remap С:\GIT\bin\libsvn_wc-1-0.dll to same address as parent -- 0x2190000
1 [main] perl 8248 sync_with_child: child 5768(0xB60) died before initialization with status code 0x1
2 [main] perl 8248 sync_with_child: *** child state child loading dlls
C:\GIT\bin\perl.exe: *** unable to remap С:\GIT\lib\perl5\site_perl\5.8.8\msys\auto\SVN\_Delta\_Delta.dll to same address as parent -- 0x630000
3 [main] perl 8248 sync_with_child: child 6632(0xB90) died before initialization with status code 0x6802752F
3 [main] perl 8248 sync_with_child: *** child state child loading dlls
```

## The problem ##
Digging the problem for some time I've found the reason of these errors on my PC (in perl's case).

  * Address, where perl wants to load its libraries was used by `C:\Windows\System32\uxtheme.dll`.
  * This `uxtheme.dll` was loaded as static linked library by `C:\Windows\apppatch\AcGenral.dll`.
  * This `AcGenral.dll` was loaded (dynamically) by `C:\Windows\System32\apphelp.dll`.
  * And this `apphelp.dll` was loaded in the parent process of the `ConEmu.exe` (that was `USBSafelyRemove.exe` in my case).
  * Oops...

## The check ##
_The following examples shows msysgit x86_

Almost all dll-s used by cygwin/msys must be loaded at high memory addresses (above 0x60000000).

For example, if you call

```
dumpbin /HEADERS С:\GIT\bin\libsvn_wc-1-0.dll
```

You will see

```
OPTIONAL HEADER VALUES
        6F1C0000 image base (6F1C0000 to 6F2E7FFF)
```

So, you you see that one of the cygwin/msys libraries is loaded in low memory address (below 0x5FFF0000)
you have a problem most probably... The easies way to check is using ProcessExplorer.

Press `Ctrl+D` (Menu `->` View `->` Lower pane view `->` DLLs) and look at ‘Base’ column in the lower pane.
If it is not visible just right-click on the DLLs list header, choose ‘Select columns’ and turn on ‘Base address’
on the ‘DLL’ tab page. And use ‘dumpbin’ to ensure that required ‘image base’ differs.

Scroll the DLLs list in the ProcessExplorer lower pane to find ‘Base+Size’ which covers desired address was shown
by ‘dumpbin’. And who was loaded that dll? You may use DependencyWalker or ImpEx to check ‘Imports’ section
of each dll loaded before. Yeah, that is not too simple but rather...

## The solution ##
1) You need to ‘rebase’ conflicting modules.

2) Alternatively you need to run your shell in different way so process tree will not include DLLs raised the conflict.
In my case, when I run ConEmu from TaskBar icon there is no `apphelp.dll` in the result process tree (from `ConEmu.exe` down to `perl.exe`).
But when I run ConEmu from ‘USB Safe Remove’ tool (automatically on connecting USB flash), `apphelp.dll` appears in the process
tree and perl is always failed...