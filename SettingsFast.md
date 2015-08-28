<font size='5'><b><a href='http://translate.google.ru/translate?hl=ru&sl=ru&tl=en&u=http%3A%2F%2Fcode.google.com%2Fp%2Fconemu-maximus5%2Fwiki%2FSettingsFast%3Fshow%3Dcontent'>SettingsFast (autotranslated Ru-En)</a></b></font>

---


<font size='5'><b>ConEmu first start</b></font>


# Abstract #
<a href='Hidden comment: 
!ConEmu is highly adaptable tool and have great number of settings.
When You displeased by default !ConEmu behaviour - go to the Settings and change them.

Most of settings may be changed via !ConEmu interface
(Settings dialog popped up by Win-Alt-P or SystemMenu).

Less number of settings may be changed manually via [Settings#Registry registry] or [Settings#ConEmu.xml ConEmu.xml] file.
'></a>

На сайт программы регулярно приходят по результатам поискового запроса «настройка ConEmu». Мне, как разработчику,
непонятно, что там может быть непонятного ;-) Вопросов не задают ни
[здесь (в комментариях)](Settings.md), ни [здесь](http://code.google.com/p/conemu-maximus5/issues/list).
В общем, если по прочтении нижеизложенного материала вопросы остаются -
[задавайте](http://code.google.com/p/conemu-maximus5/issues/list).

Про настройку ConEmu хочется сказать следующее
  * ConEmu работает так, как хочется пользователю (то есть вам),
  * Если это не так, значит в диалоге настроек есть флажок, который вы не включили ;)
  * Конечно, может так случиться, что нужная вам опция еще никому не понадобилась, тогда вам [сюда](http://code.google.com/p/conemu-maximus5/issues/list)

# Первый запуск #
Итак, при **первом** запуске программы вы, скорее всего, увидите диалог **быстрой** настройки:

<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuPreConfig.png' title='Окно предварительной настройки ConEmu'>

Вы можете выбрать, что будете разрешать ConEmu, т.к. на некоторые функции<br>
(перехват клавиатуры, внедрение в процессы, доступ к интернету)<br>
могут ругаться антивирусные программы. <b>ConEmu не содержит троянов или вирусов</b>, код открыт, можете<br>
<a href='http://code.google.com/p/conemu-maximus5/source/checkout'>проверить</a>. Тем не менее, вы можете<br>
отключить эти функции, хотя это и не рекомендуется.<br>
<br>
<h2>Install keyboard hooks</h2>
Если флажок отключить, ConEmu не сможет обрабатывать некоторые<br>
клавиатурные комбинации. Например, в Windows 7 комбинации Win+<i>цифра</i> (Win+1, Win+2, т.д.)<br>
переключают/запускают программы на панели задач. А ведь ими удобно переключать табы в окне ConEmu.<br>
Включение флажка «Install keyboard hooks» означает не то, что ConEmu будет блокировать системные<br>
клавиатурные комбинации, а только то, что у вас есть возможность включить или отключить их перехват<br>
в диалоге настроек. Например, флажками «Win+Number - activates console», «Win+Tab - switch consoles (Tabs)»<br>
и т.п. (вкладка «Keys» диалога настроек).<br>
<br>
<h2>Inject ConEmuHk.dll into processes, started in ConEmu tabs</h2>
Для многих возможностей<br>
(обработку параметра "-new_console", работу графических приложений во вкладках ConEmu, ...)<br>
и избежания проблем (например, появление графических диалогов или меню под окном ConEmu, ...)<br>
требуется внедрение в запускаемые процессы библиотеки ConEmuHk.dll<br>
(или ConEmuHk64.dll для 64-битных приложений).<br>
<br>
<h2>Enable automatic updates</h2>
Все очевидно, разрешить обновление программы из интернета.<br>
Потом, в диалоге настроек на вкладке «Update» можно будет настроить параметры обновления.<br>
ConEmu не «самовольничает», перед запуском процесса обновления у вас спросят подтверждение.<br>
<br>
<h2>Disable ConIme.exe autorun (Vista only)</h2>
Пользователи Windows Vista могут увидеть этот флажок. Почему и зачем написано в <a href='http://conemu.github.io/en/ConEmuFAQ.html#Windows_Vista'>FAQ</a>.<br>
<br>
<h2>После нажатия кнопки «OK»</h2>
ConEmu запускает в своей вкладке либо Far Manager (если найден),<br>
либо cmd.exe, либо ту программу (или программы), которые указаны в свойствах ярлыка<br>
или в настройке ConEmu.<br>
<br>
<h1>Теперь можно открыть диалог настроек</h1>
<a href='Settings#Settings_dialog.md'>Как открыть диалог настроек</a>

<h1>Ссылки на документацию</h1>
<ul><li><a href='http://conemu.github.io/en/Settings.html'>Настройки</a>
</li><li><a href='http://conemu.github.io/en/Command_Line.html'>Параметры командной строки, запуск при старте нескольких вкладок</a>
</li><li><a href='http://conemu.github.io/ru/ConEmuFAQ.html'>ЧаВо (на русском)</a>
</li><li><a href='http://conemu.github.io/en/TableOfContents.html'>Остальная документация</a>