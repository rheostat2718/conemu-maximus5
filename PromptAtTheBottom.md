Do you want to get your cmd.exe prompt bottom aligned? On ConEmu startup and after "cls"?

## Far Manager ##
Actually, I can't understand why anyone use cmd as shell but not a FarManager.
It's a fast handy and powerfull combination of:

  * command prompt (you may run any console or GUI application from its prompt, any %COMSPEC% commands, can be integrated with powershell);
  * two pane file manager (copy/move files, create directories and hard links, browse remote hosts, and so on); you have no need in file manager? are you sure you are working on your PC?
  * powerful and fast editor with syntax highlighting;
  * dozens of miscellaneous plugins for anyone taste and purpose;

Try it! [Download](http://www.farmanager.com/download.php?l=en) and install into ConEmu's folder
(put far.exe near to ConEmu.exe) and run it.
If you don't need panels on startup (really?) - press **Ctrl+O**, **Ctrl+B** and save configuration changes **Shift+F9**.
You will get command prompt with permanent history (Alt+F8).


## Plain cmd.exe ##
OK. If you still want use plain "cmd.exe" - here is one trick using [AnsiEscapeCodes](http://conemu.github.io/en/AnsiEscapeCodes.html).

### Set up your task ###
<img src='http://conemu-maximus5.googlecode.com/svn/files/PromptAtBottom1.png' alt='Prompt at the bottom' title='Prompt at the bottom of ConEmu window'>

<h3>Choose it as startup task</h3>
<img src='http://conemu-maximus5.googlecode.com/svn/files/PromptAtBottom2.png' alt='Prompt at the bottom' title='Prompt at the bottom of ConEmu window'>

<h3>Prompt will appears at the bottom</h3>
<img src='http://conemu-maximus5.googlecode.com/svn/files/PromptAtBottom3.png' alt='Prompt at the bottom' title='Prompt at the bottom of ConEmu window'>