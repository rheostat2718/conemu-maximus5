### ConEmu plugin macros ###
#### Far2 ####
  * callplugin(0x43454D55,"_GuiMacro_")
  * callplugin(0x43454D55,_PluginCommand_)
#### Far3 (Plugin.Call is alias for callplugin) ####
  * Plugin.Call("4b675d80-1d4a-4ea9-8436-fdc23f2fc14b","_GuiMacro_")
  * Plugin.Call("4b675d80-1d4a-4ea9-8436-fdc23f2fc14b",_PluginCommand_)
#### Where PluginCommand may be ####
| 1 | Edit console output |
|:--|:--------------------|
| 2 | View console output |
| 3 | Switch tab visibility |
| 4 | Switch next tab     |
| 5 | Switch previous tab |
| 6 | Switch tab commit   |
| 7 | Attach to ConEmu    |
| 8 | Start ConEmu debug  |


### ConEmu Panel Views plugin macros ###
#### Far2 ####
  * callplugin(0x43455568,_PluginCommand_)
#### Far3 (Plugin.Call is alias for callplugin) ####
  * Plugin.Call("bd454d48-448e-46cc-909d-b6cf789c2d65",_PluginCommand_)
#### Where PluginCommand may be ####
| 1 | Switch Thumbnail view on active panel |
|:--|:--------------------------------------|
| 2 | Switch Tiles view on active panel     |