[Download (Far2 x86/x64)](http://code.google.com/p/conemu-maximus5/downloads/list?q=summary:Selector)

### About ###
Данный плагин позволяет открывать папки и позиционировать курсор на файл используя одну копию Far Manager.

### Requirements ###
Минимальная поддерживаемая версия FAR 2 build 1007.

### Installation ###
  1. Распакуйте архив в папку с far.exe;
  1. Замените вызовы far.exe на FarS.exe.

### How it works? ###
При запуске FarS.exe ищет открытые и доступные копии FAR.EXE.
Недоступный - значит нет активных панелей (в данный момент выполняется какая-либо консольная команда, активен редактор/вьювер, или диалог...).

Если доступных копий нет вообще - запускается новый FAR.EXE, или новая вкладка в ConEmu если он найден в той же папке, что и FAR.EXE.

Если доступные копии есть - дальнейшее поведение зависит от настройки:
```
[HKEY_CURRENT_USER\Software\Far2\Plugins\FarSelector]
"Autoselect"=hex:01
```
Если 00 - открывается диалог выбора копии с информацией о папках на панелях.
Иначе - параметры сразу передаются в первую (активную) копию FAR.EXE.

Применяются параметры макросом вроде:
```
$If (Shell) panel.SetPath("C:\\Program Files\\far\\","far.exe") CtrlPgDn $End
```

CtrlPgDn "нажимается" только для первого файла (активная панель).
Добавление CtrlPgDn в макрос можно отключить в настройке плагина.

## Screenshots ##
**Optional instance selection dialog**<br>
<img src='http://conemu-maximus5.googlecode.com/svn/files/FarSelector/fars.png' title='FAR Selector. Optional instance selection dialog'>

<b>Plugin configuration</b><br>
<img src='http://conemu-maximus5.googlecode.com/svn/files/FarSelector/config.png' title='FAR Selector. Plugin configuration'>