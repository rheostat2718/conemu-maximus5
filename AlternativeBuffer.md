#summary Output pause ability.

ConEmu may "pause" your ConsoleApplication output.
Just press Win+A or press the "lock" button on the ToolBar.

ConEmu will load the entire console back-scroll buffer in memory (that may
takes few seconds in some cases) and display it. If any ConsoleApplication
will change output in the RealConsole at that time, changes will not be
displayed in ConEmu until you exit AlternativeBuffer by pressing Esc or
Win+A.