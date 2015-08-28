#summary Console title vs tab label

There are two things

a) Console title. Well, you can set it from ConEmu task, but that title will be overrided/changed in the most cases by your shell.

> title "Your cool tab" & cmd

b) Tab title. By default it shows console title with console number as prefix. It can be configured by templates on the SettingsTabBar page. Also, it can be renamed by user (Apps+R or tab menu). And of course it can be specified from your task. Read wiki NewConsole switch `t`

> cmd -new\_console:t:"Your cool tab"