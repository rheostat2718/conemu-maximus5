ConEmu ‘Keys & Macro’ page contains few hotkeys marked as ‘Global’. Thought, most useful is ‘Minimize/Restore’.
These hotkeys are registered globally in system via
[RegisterHotKey](http://msdn.microsoft.com/en-us/library/windows/desktop/ms646309(v=vs.85).aspx)
and are available even ConEmu has no focus.

Because these hotkeys are processed by Windows, some limitations applied:

  * You can't use ‘Apps’ as modifier.
  * Left and Right modifiers are indifferent.
  * You can't register hotkey already registered by any other application.
  * The F12 key is [reserved for use by the debugger](http://msdn.microsoft.com/en-us/library/windows/desktop/ms646309(v=vs.85).aspx) at all times, so it should not be registered as a hot key. Even when you are not debugging an application, F12 is reserved in case a kernel-mode debugger or a just-in-time debugger is resident.

## Using F12 as global hotkey ##
As [msdn](http://msdn.microsoft.com/en-us/library/windows/desktop/ms646309(v=vs.85).aspx) said, F12 is reserved for use by the debugger.

```
[HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\AeDebug]
"UserDebuggerHotKey"=dword:00000000 ; The default, steal F12 global hotkey
```

However, [technet](http://technet.microsoft.com/en-us/library/cc786263.aspx) describes a way how to choose another key via registry.
For example, change the value to `0x13` to use `Pause` key for debugger.

|**Code**|**Name**       |**Code**|**Name**          |**Code**     |**Name**       |
|:-------|:--------------|:-------|:-----------------|:------------|:--------------|
| 0x00   | F12           | 0x15   | VK\_KANA, ...    | 0x25        | VK\_LEFT      |
| 0x01   | VK\_LBUTTON   | 0x17   | VK\_JUNJA        | 0x26        | VK\_UP        |
| 0x02   | VK\_RBUTTON   | 0x18   | VK\_FINAL        | 0x27        | VK\_RIGHT     |
| 0x03   | VK\_CANCEL    | 0x19   | VK\_HANJA, ...   | 0x28        | VK\_DOWN      |
| 0x04   | VK\_MBUTTON   | 0x1B   | VK\_ESCAPE       | 0x29        | VK\_SELECT    |
| 0x08   | VK\_BACK      | 0x1C   | VK\_CONVERT      | 0x2A        | VK\_PRINT     |
| 0x09   | VK\_TAB       | 0x1D   | VK\_NONCONVERT   | 0x2B        | VK\_EXECUTE   |
| 0x0C   | VK\_CLEAR     | 0x1E   | VK\_ACCEPT       | 0x2C        | VK\_SNAPSHOT  |
| 0x0D   | VK\_RETURN    | 0x1F   | VK\_MODECHANGE   | 0x2D        | VK\_INSERT    |
| 0x10   | VK\_SHIFT     | 0x20   | VK\_SPACE        | 0x2E        | VK\_DELETE    |
| 0x11   | VK\_CONTROL   | 0x21   | VK\_PRIOR        | 0x2F        | VK\_HELP      |
| 0x12   | VK\_MENU      | 0x22   | VK\_NEXT         | 0x30–0x39   | (ASCII 0–9)   |
| 0x13   | VK\_PAUSE     | 0x23   | VK\_END          | 0x41–0x5A   | (ASCII A–Z)   |
| 0x14   | VK\_CAPITAL   | 0x24   | VK\_HOME         |             |               |