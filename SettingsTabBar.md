#summary This page was generated automatically from ConEmu sources
<a href='Hidden comment:  IDD_SPG_TABS '></a>
# Settings: Tabs #
<img src='http://conemu-maximus5.googlecode.com/svn/files/Settings-TabBar.png' title='ConEmu Settings: Tabs'>



<h2>Tabs (panels, editors, viewers)</h2>



<ul><li><b>Always show</b>
</li><li><b>Auto show</b>
</li><li><b>Don't show</b></li></ul>


<b>Tabs on bottom</b>

<b>Internal CtrlTab</b> Handle CtrlTab and CtrlShiftTab internally (by ConEmu). These keystrokes will not be sent to console window, but You can easily switch between consoles (panels).<br>
<br>
<b>Lazy tab switch</b> When checked - real window switching will be performed on Ctrl depress<br>
<br>
<b>Recent mode</b> Switch first between recent tabs. You may still switch between tabs in standard manner using Left/Right (after CtrlTab), while Ctrl is still presses.<br>
<br>
<b>Hide disabled tabs</b> Hide tabs, wich can't be activated. E.g. hide Far Manager editors while executing cmd.exe<br>
<br>
<b>Far windows</b> Show all Far Manager windows (panels, editors, viewers) instead of one tab for one console<br>
<br>
<b>‘Host-key’+Number iterates Far windows</b> Iterate opened Far windows with ‘Host-key’+Number<br>
<br>
<b>Active console only</b> Show tabs from active console only<br>
<br>
<b>One tab per group</b> Show only one tab for all splits in the group<br>
<br>
<b>Activate split on mouse over</b> When several panes (splits) are visible simultaneously activate console with mouse over, 3rd-state - match ‘Active window tracking’ system settings<br>
<br>
<br>
<br>
<h2>Tabs font</h2>



RTEXT<br>
<br>
<br>
<br>
<h2>Tab templates</h2>





<b>%s - Title, %c - Console #, %n - Active process name, %p - PID, %a - ‘Admin’, %% - %</b>



<b>Template</b> Common tab template (any console program except Far Manager)<br>
<br>
LTEXT<br>
<br>
<br>
<br>
<b>Skip words from title</b>

<b>Maximum tab width (in chars)</b>

<b>Admin shield</b> When this is checked - ‘Shield’ icon will be shown in tabs, started ‘As administrator’<br>
<br>
<b>suffix</b> When this is checked - specified suffix will be appended in tabs titles, started ‘As administrator’. You may choose insertion place with ‘%a’ var, otherwise suffix will be at the tab end.<br>
<br>
<br>
<br>
<br>
<br>
<h2>Tab double click actions</h2>



LTEXT<br>
<br>
<br>
<br>
RTEXT<br>
<br>
<br>
<br>
When you double click on the tab...<br>
<br>
When you double click on the free space of tab bar... ‘Auto’ means ‘Maximize/Restore’ when caption is hidden and ‘Open new shell’ when caption is visible<br>
<br>
