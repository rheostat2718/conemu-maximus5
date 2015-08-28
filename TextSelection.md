Under construction, [Issue 887](https://code.google.com/p/conemu-maximus5/issues/detail?id=887), [Issue 1292](https://code.google.com/p/conemu-maximus5/issues/detail?id=1292), [Issue 1332](https://code.google.com/p/conemu-maximus5/issues/detail?id=1332)

ConEmu allows marking console block (like real console **Mark** menu item, or Far Manager **AltIns** feature)
and text (like standard GUI text editors), copying marked text to Windows Clipboard,
pasting from Clipboard to console.

**Note**. Копирование выполняется 'с экрана', то есть с выделением в редакторе Far Manager не имеет ничего общего.

<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsSelection.png' title='ConEmu text selection settings'>

<ul><li>Выделение теперь может быть 'блочным' (как в обычной консоли) или 'текстовым' (как в текстовых редакторах).<br>
</li><li>В меню окна добавлены пункты 'Mark block' и 'Mark text'.<br>
</li><li>Добавлена настройка клавиш мышки и модификаторов. Теперь можно указать когда разрешено выделение мышкой:<br>
<ul><li>Только при наличии прокрутки (Buffer only, по умолчанию), или всегда (Always).<br>
</li><li>Можно указать модификатор (Alt, Shift, ...) для обоих типов выделения (Block & Text).<br>
</li><li>Можно выбрать действия для правой и средней кнопки мышки (Copy/Paste).<br>
</li><li>Можно настроить цвет фона и текста для выделения.<br>
</li></ul></li><li>Снято ограничение на флажок 'Mouse' в FAR (его можно оставлять включенным).</li></ul>

<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuBlockSelection.png' title='ConEmu Block selection (like real console)'>

<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuTextSelection.png' title='ConEmu Text selection (like text editors)'>