﻿#summary ConEmu.xml, how to send your config to developer

As [wiki](https://code.google.com/p/conemu-maximus5/wiki/Settings#Where_settings_are_stored) said,
ConEmu settings may be stored in the ‘Windows Registry’ and in the xml files (`ConEmu.xml` usually).

When one reports about problem, developer needs your ConEmu settings often.

The easiest (and may be most proper) way to provide it is ‘Export...’ button in the
[ConEmu Settings dialog](https://code.google.com/p/conemu-maximus5/wiki/Settings#Settings_dialog).

Save your config in the new xml (**don't overwrite your existing config**) file and attach it to the created issue.

## Note for PortableApps.com format ##
When you are running ConEmu installed with ‘paf’ bundle,
your settings will be stored in `<app>/Data/settings/ConEmu.xml`.