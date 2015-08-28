<font size='5'><b><a href='http://translate.google.ru/translate?hl=ru&sl=ru&tl=en&u=http%3A%2F%2Fcode.google.com%2Fp%2Fconemu-maximus5%2Fwiki%2FWhats_New_1%3Fshow%3Dcontent'>Whats_New_1 (autotranslated Ru-En)</a></b></font>

---


## Newer entries ##
  * [Builds 140104 .. current](Whats_New.md)
  * [Builds 120101 .. 131225](Whats_New.md)


## Build 111220 ##
  * Улучшен детектор прогресса в консоли.
  * GuiMacro. Новая функция: `Shell("<Verb>","<File>"[,"<Parms>"[,"<Dir>"[,<ShowCmd>]]])`
```
    Verb - "open", "print", и т.п. или специальный "new_console"
    Примеры (для диалога ConEmu GUI Macro):
      shell("open","cmd","/c dir c:\ -new_console:b")
    Примеры (для макросов Far 3.x):
      callplugin("4b675d80-1d4a-4ea9-8436-fdc23f2fc14b","shell(\"open\",\""+env("FARHOME")+"\\Encyclopedia\\FarEncyclopedia.ru.chm\")")
      callplugin("4b675d80-1d4a-4ea9-8436-fdc23f2fc14b","shell(\"new_console\",\""+env("FARHOME")+"\\far.exe\")")
```
  * На вкладке 'Debug' окна настроек добавлена возможность журналирования выполняемых команд.
  * Некорректно вырезался '-new\_console' при наличии кавычки после параметра.


## Build 111212 ##
  * Продолжение GUI приложений во вкладках.
  * ConEmu64 & Far x86: при выходе из Far могло появляться сообщение 'Press Enter or Esc to close console'.
  * [Issue 462](https://code.google.com/p/conemu-maximus5/issues/detail?id=462): ConEmu 111206 & right click.
  * [Issue 461](https://code.google.com/p/conemu-maximus5/issues/detail?id=461): При максимизации внутренняя консоль не максимизируется.
  * Расширены возможности запуска новой консоли: `-new_console[:bh[N]caru[:user:pwd]]`
    * `b` - Create background tab
    * `h<height>` - пример, h0 - отключить буфер, h9999 - включить буфер в 9999 строк
    * `n` - отключить "Press Enter or Esc to close console"
    * `c` - принудительно включить "Press Enter or Esc to close console"
    * `a` - RunAs shell verb (as admin on Vista+, login/password in Win2k and WinXP)
    * `r` - Run as restricted user
    * `u` - ConEmu choose user dialog
    * `u:<user>:<pwd>` - specify user/pwd in args, MUST BE LAST OPTION


## Build 111206 ##
  * Продолжение GUI приложений во вкладках.
  * Sources: класс CRealConsole разделен на два объекта.
  * Правки в запускаторе.
  * Исходники ConEmu проверены анализатором PVS-Studio.
  * При включении "Shell and processes" в логе отображается содержимое bat/cmd/tmp файлов длинной менее 32K. Можно использовать для отлова ошибок параметров компиляции/линковки программ.
  * Не мигаем курсором если Far занят (выполняет длинный макрос, ищет по сети, грузит файлы и т.п.)
  * [Issue 459](https://code.google.com/p/conemu-maximus5/issues/detail?id=459): обработка клавиши Win.


## Build 111120 ##
  * Изменен внешний вид диалога настроек.
  * Небольшие правки обработки кликов по миниатюрам панели задач Windows 7.
  * Исправлен Alt-Tab, Alt-Tab для Win2k/WinXP при включенном Tabs->'Show on taskbar'.
  * Исправлена подсказка к флажку Tabs->'Active only'.
  * x86 и x64 исталляторы объеденены в одном файле: ConEmuSetup.??????.exe.


## Build 111117 ##
  * В Far3 не работало переключение окон при открытом QView.
  * Продолжение подготовки к переходу на новые табы (исправлены появившиеся в 111110 ошибки)
    * поломалась зарисовка полей вокруг консоли.
    * [Issue 456](https://code.google.com/p/conemu-maximus5/issues/detail?id=456): в некоторых случаях не обновлялся или не отрисовывался заголовок окна.
    * [Issue 455](https://code.google.com/p/conemu-maximus5/issues/detail?id=455): возможно, исправлена ошибка отрисовки при переключении в другое приложение.
  * Изменен адрес загрузки ConEmuHk.dll/ConEmuHk64.dll.
  * Sources: класс настроек разделен на два объекта.


## Build 111110 ##
#### Changes ####
  * [Issue 449](https://code.google.com/p/conemu-maximus5/issues/detail?id=449): Resizing tabs with mouse in 'panel view' (thumbs/tiles) does not work.
  * [Issue 451](https://code.google.com/p/conemu-maximus5/issues/detail?id=451): падение при загрузке background из png.
  * Settings->Tabs->'Show on taskbar' - отображение на панели задач всех запущенных консолей. Если флажок в 3-м состоянии - консоли отображаются только в Vista и выше. В Windows 7 добавлена поддержка Aero thumbnails & peeks.
  * msi: не запоминался путь, заданный для ExtendedConsole.
  * Начало подготовки к переходу на новые табы.
  * В диалоге настроек добавлены кнопки 'Reset' (сброс к настройкам по умолчанию) и 'Reload' (перезагрузить из xml/reg).
  * Правки в запускаторе.


## Build 111101 ##
#### Changes ####
  * В 111019 поломался TrueMod в Far2.
  * Продолжение GUI приложений во вкладках.


## Build 111031 ##
#### Changes ####
  * [Issue 443](https://code.google.com/p/conemu-maximus5/issues/detail?id=443): Jpeg and PNG in Background image.
  * [Issue 319](https://code.google.com/p/conemu-maximus5/issues/detail?id=319): Поломался переход в редактор по клику для 'test.c(50): error: ...'


## Build 111027 ##
#### Changes ####
  * [Issue 440](https://code.google.com/p/conemu-maximus5/issues/detail?id=440): Игнорировалось наличие плагина ConEmu.x64.dll если Far.exe был переименован.
  * [Issue 441](https://code.google.com/p/conemu-maximus5/issues/detail?id=441): ConEmu 2011.10.19 - сломался QView + PictureVew2.
  * Диалог создания/пересоздания консоли.
> > 'Команда по умолчанию' - то, что запускается при старте ConEmu (например "C:\Far\Far.exe").
> > 'Текущая команда' - то, что запущено в текущей консоли  (например "C:\Far\Far64.exe /w").
> > При создании новой консоли в диалоге отображается 'Команда по умолчанию', 'Текущая команда'
> > добавляется в историю (выпадающий список) следующим пунктом. Таким образом, поведение ConEmu
> > при отображении диалога теперь соответствует 'тихому' созданию новой консоли ([ ] Create confirmation).
> > При пересоздании активной консоли в диалоге отображается 'Текущая команда', а 'Команда по умолчанию'
> > добавляется в историю (выпадающий список) следующим пунктом.
  * Для некоторых пунктов меню (таба и системного) начали всплывать подсказки.
  * Вернул slideshow для PictureView2 (кнопка Pause, интервал '-' и '+' на _основной_ клавиатуре).
  * Продолжение GUI приложений во вкладках.


## Build 111026 ##
#### Changes ####
  * [Issue 438](https://code.google.com/p/conemu-maximus5/issues/detail?id=438): Far мог падать при попытке настройки палитры TrueColor (ExtendedConsole64.dll) в режиме "Far /w".
  * В 111019 поломался плагин ConEmu Panel Views.
  * При включенном флажке 'Save window size and position on exit' сохряняем один раз, перед закрытием консолей.
  * Разработчикам Far-плагинов. ConEmu.dll: HWND WINAPI GetFarHWND2(int anConEmuOnly)
```
	anConEmuOnly
		2 - вернуть главное окно !ConEmu
		1 - вернуть окно отрисовки
		0 - если в !ConEmu - вернуть окно отрисовки, иначе - вернуть окно консоли
```
  * Еще немного FAQ-ConEmu.txt


## Build 111024 ##
#### Changes ####
  * В 111023 поломался диалог 'Recreate console'.
  * Продолжение GUI приложений во вкладках.


## Build 111023 ##
#### Changes ####
  * Ресайз окна «HostKey»+стрелки не работал, если «HostKey» был переопределен (не Win).
  * При включенном флажке '«User screen» transparency' часть картинки (PicView/MMView) могла оставаться невидимой.
  * [Issue 423](https://code.google.com/p/conemu-maximus5/issues/detail?id=423): При просмотре картинок (PicView и т.п.) временно отключается альфа-прозрачность.
  * Макросы в Far могли не выполняться, пока мышкой не дернешь над консолью.
  * ConEmu падал, если все консольные процессы завершались пока было открыто окно 'Recreate console'.
  * Обновлен FAQ-ConEmu.txt


## Build 111019 ##
#### Changes ####
  * Большие внутренние изменения. Для каждой виртуальной консоли создается свое окно отрисовки.
    * Функция GetConsoleWindow перехватвается, и возвращает окно отрисовки ConEmu.
    * Если плагинам/программам реально требуется дескриптор окна консоли - ConEmuHk.dll: GetConEmuHWND(2).
    * Функции GetParent, GetWindow, GetAncestor перехватываются, и для окна отрисовки подставляют результат главного окна ConEmu.
    * Враппер для старых версий PicView/!MMView больше не нужен.
  * [Issue 393](https://code.google.com/p/conemu-maximus5/issues/detail?id=393): Запуск второго экземпляра Far внутри ConEmu под высокой нагрузкой.
  * [Issue 431](https://code.google.com/p/conemu-maximus5/issues/detail?id=431): Падение при закрытии окна настройки плагина 'ConEmu Background'.
  * [Issue 434](https://code.google.com/p/conemu-maximus5/issues/detail?id=434): Опять работаем в Windows 2000.
  * При 'RightClick 4 context menu' в Far 1.7x гасились панели.
  * С какой-то сборки отвалился Drop в командную строку.
  * При отсутствии файла ConEmuCD.dll/ConEmuCD64.dll в консоль выводились 'иероглифы' вместо текста ошибки при попытке выполнения команд.
  * В некоторых случаях после старта консоли 'Run as admin' не инициализировалась консоль (не отображалось содержимое).
  * Теперь во вкладке ConEmu можно запускать некоторые графические приложения: Putty, Notepad, TC ;) При этом кнопка 'буферного режима' скрывает/отображает окно приложения.
  * В окне 'Settings' добавлена вкладка 'Keys' на которой показаны все работающие в ConEmu кнопкосочетания. Туда же перемещены настройки 'Host-key' и 'Send Alt+Enter', 'Send Alt+Space'.
  * Добавлена возможность перехвата некоторых системных кнопкосочетаний (AltEsc, AltTab, CtrlEsc, PrntScrn). Настройка на вкладке 'Keys'. Альтернативно поддержан Far плагин XKeys.
  * На вкладке 'Keys' добавлен флажок 'Win+Arrows' разрешающий ресайз окна ConEmu стрелками на клавиатуре.
  * Уменьшен объем (и соответственно время) передаваемых данных при отрисовке фона плагинами. Использовать старый PanelColorer не рекомендуется.
  * Оптимизация детектора диалогов.
  * Небольшое изменение скрытия 'автоматической' полосы прокрутки. Если курсор мыши вне окна консоли - скрытие не выполняется, во избежание мелькания полосы прокрутки при вялотекущих процессах в консоли.
  * При Close/Recreate консоли сразу отключается Background, установленный плагинами.
  * Системное меню и меню таба.
    * Меню приведены к единому виду.
    * Добавлен пункт "Attach to...", позволяющий подцепиться к приложению или консоли (не закончено).
    * В системное меню добавлено субменю 'Active console', содержащее пункты 'Close', 'Detach', и т.д.
    * ... добавлено субменю 'Console list', отображающее список открытых консолей.
  * Описана структура и параметры в файле 'Background.xml'.
    * Добавлена возможность задания цветов картинок и полосы свободного места.
    * ... задания размера полосы свободного места.
    * ... задания разных цветов панелей в зависимости от буквы диска.
  * FindFirstChangeNotification для проверки изменений в Background.xml (плагин ConEmu Background).
  * Сообщение о невозможности активации таба отображается под центром таба. Также не пытаемся активировать табы, если они заблокированы диалогом.


## Build 110926 ##
#### Changes ####
  * Пункт меню 'Terminate' и кнопка 'Terminate' в 'Recreate dialog' отображает подтверждение с выбором: закрыть консоль 'крестиком' или убить активный процесс (TerminateProcess).
  * Упрощена настройка Background. В поле 'Replace color indexes' можно указать `"*"` (звездочку), тогда будет краситься только фон панелей Far manager.
  * Far3 build 2204.


## Build 110925 ##
#### Bugfixes ####
  * Плагины 'ConEmu Background' и плагин 'ConEmu Underlines' не работали в Far 1.7x.
#### Changes ####
  * Настройка плагина 'ConEmu Background'. Можно задать полный путь к xml файлу, например "%FARHOME%\Background.xml".


## Build 110924 ##
#### Bugfixes ####
  * [Issue 158](https://code.google.com/p/conemu-maximus5/issues/detail?id=158): ConEmu PanelViews в некоторых случаях конфликтовал с плагином PanelTabs (не реагировал на клавиатуру и мышь).
#### Changes ####
  * Опция "MainTimerInactiveElapse" - периодичность обновления неактивной консоли.
  * Плагин 'ConEmu Underlines' теперь умеет подсвечивать панели черезстрочными полосами.
  * Добавлен новый плагин 'ConEmu Background' аналог неподдерживаемого [PanelColorer](ConEmuFarPlugin#PanelColorer.md).


## Build 110914 ##
#### Bugfixes ####
  * [Issue 427](https://code.google.com/p/conemu-maximus5/issues/detail?id=427): После AltEnter, AltEnter некорректно сохранялся текущий размер окна при выходе.
  * [Issue 429](https://code.google.com/p/conemu-maximus5/issues/detail?id=429): Win-W won't works in Windows 8.
#### Changes ####
  * [Issue 425](https://code.google.com/p/conemu-maximus5/issues/detail?id=425): Выделение слов двойным кликом, как в обычной консоли.
  * Теперь можно выделять текст в фаровском CtrlO из редактора. Есть, конечно, AltIns, но тем не менее.
  * Добавлена авторегистрация шрифтов в `*.otf` файлах.
  * [Issue 319](https://code.google.com/p/conemu-maximus5/issues/detail?id=319): Из консоли фара можно открывать как ссылки ошибки компиляторов (вроде ConEmu.cpp(1999): error: xxx).


## Build 110914 ##
#### Bugfixes ####
  * DosBox не запускался, если ConEmu установлен в 'Program Files' (путь к DosBox содержал пробелы).
  * После нескольких вызовов окна настроек в списке шрифтов задваивались шрифты вида '`[Raster Fonts 8x12]`'.
#### Changes ####
  * [Issue 426](https://code.google.com/p/conemu-maximus5/issues/detail?id=426): Режим 'Auto' для правой и средней кнопок мыши, копирование - если выделен текст, иначе - вставка.
  * Minor changes in hookset (32`<->`64bit).
  * ExtendedConsole: немного улучшено отображение цветов в RealConsole.
  * Far3 build 2194.


## Build 110903 ##
#### Bugfixes ####
  * Far3 build 2184.
  * Изменения в модуле ConEmuHk.
  * Плагин 'ConEmu Underlines' валил Far.


## Build 110826 ##
#### Changes ####
  * Far3 build 2174.


## Build 110824 ##
#### Bugfixes ####
  * В некоторых случаях, при отображении меню (RightClick 4 context menu) гасились панели.
#### Changes ####
  * Far3 build 2159.
  * [Issue 422](https://code.google.com/p/conemu-maximus5/issues/detail?id=422): Настройка 'DisableMouse'. Через реестр/xml можно отключить посылку мышиных событий в консоль. Некоторые опции ('Resize panels by mouse', 'Select text with mouse', и т.п.) отключаются отдельно.


## Build 110811 ##
#### Changes ####
  * Изменено отображение непечатных (combining) символов: accents, tildas, dot above, xFEFF и т.п. Обычно эти символы при печати располагаются над предыдущей буквой. Однако, для 'консольной ясности' теперь они отображаются в своем знакоместе.


## Build 110809 ##
#### Bugfixes ####
  * Поправлено определение координат панелей для многострочной и отключенной области статуса.
#### Changes ####
  * Доработан 'RightClick for context menu'. Курсор на файл ставится под кликом сразу (при начале длинного клика).
  * [Issue 420](https://code.google.com/p/conemu-maximus5/issues/detail?id=420): Option 'Win-Tab' on Tabs page for tab switching in Windows 7.


## Build 110808 ##
#### Bugfixes ####
  * Падение в некоторых случаях (например при очистке корзины через EMenu).
  * Не работал Fade для расширенных (24bit) цветов.
#### Changes ####
  * ExtendedConsole: отображение в диалоге Bold/Italic/Underline; включение флажка '4bit' сразу показывает измененный цвет.


## Build 110807 ##
#### Bugfixes ####
  * После Attach (например из плагина) окно консоли не скрывалось.
  * [Issue 418](https://code.google.com/p/conemu-maximus5/issues/detail?id=418): Не работал макрос FAR\_AutoAttach.reg в Far2.
#### Changes ####
  * В меню 'Debug' (системное или контекстное меню таба) добавлен пункт 'Debug active process'.
  * В поставку добавлены ExtendedConsole.dll и ExtendedConsole64.dll - поддержка 24bit цветов и Bold/Italic/Underline в Far3.


## Build 110805 ##
#### Bugfixes ####
  * Отрисовка TrueMod в 'far /w'.
  * Умеем восстанавливать разрушенное другими программами системное меню.
  * [Issue 387](https://code.google.com/p/conemu-maximus5/issues/detail?id=387), [Issue 397](https://code.google.com/p/conemu-maximus5/issues/detail?id=397): Диагностика ошибок - отсутствие необходимых 'ConEmuC.exe' или 'ConEmuC64.exe'.
  * [Issue 414](https://code.google.com/p/conemu-maximus5/issues/detail?id=414): Поломался аттач после детача по CtrlAltTab.
  * [Issue 416](https://code.google.com/p/conemu-maximus5/issues/detail?id=416): Настройка "DownShowHiddenMessage". Иконка из TSA автоматически убирается, если окно опять стало видимым.
  * [Issue 417](https://code.google.com/p/conemu-maximus5/issues/detail?id=417): Исправлен глюк Far плагина 'Color Picker'.


## Build 110731 ##
#### Bugfixes ####
  * Небольшая доработка TrueMod support.


## Build 110721 ##
#### Bugfixes ####
  * В версии 110720 в некоторых случаях Far аварийно завершался.


## Build 110720 ##
  * Плагины не будут работать в Far3 выше 2102.
#### Bugfixes ####
  * Не работал перехват WinAPI в плагинах, загруженых через Far3wrap.
#### Changes ####
  * В Thumbnails & Tiles учитывается многострочная статусная область панелей.
  * Добавлен файл `ConEmu\Far3_reg\Thumbnails_KeyBar.xml`


## Build 110712 ##
#### Bugfixes ####
  * Полный путь в заголовках табов редакторов/вьюверов.
#### Changes ####
  * При удерживании модификатора выделения текста (настройка 'Select text with mouse') движение мышки не пересылается в консоль.
  * В системное меню добавлен пункт 'Bring here' - подвинуть окно ConEmu в левый верхний угол текущего монитора (если вдруг окно уехало за пределы видимости).


## Build 110706 ##
#### Bugfixes ####
  * [Issue 411](https://code.google.com/p/conemu-maximus5/issues/detail?id=411): Ошибки Drag&Drop в коммандную строку.
  * В плагине не выполнялся пункт 'Start ConEmu debug'.
#### Changes ####
  * Расширены возможности встроенного дебаггера ('Start ConEmu debug' из плагина, или 'Debug log (GUI)' из системного меню ConEmu).
    * При нажатии CtrlBreak в окне дебаггера предлагается создать minidump.
    * В заголовке дебаггера отображается PID отлаживаемого процесса.
  * Доработки для Far3 build 2082.
  * В сообщении 'ConEmu received old version packet' отображается полный путь процесса или модуля, приславшего пакет некорректной версии.
  * В шаблонах заголовков табов (Tab templates) можно использовать макроподстановки: %s - заголовок консоли, %i - номер окна в Far (0-based), %p - PID активного процесса в консоли, %c - номер консоли (1-based).
  * Если какая-то другая программа скрыла окно ConEmu - в TSA отображается иконка и всплывающее сообщение об этом безобразии.


## Build 110629 ##
#### Bugfixes ####
  * [Issue 409](https://code.google.com/p/conemu-maximus5/issues/detail?id=409): "Far.exe /e newfile" из ConEmu: Exception при выходе. Требуется версия Far 2.1 bis12 или выше.
#### Changes ####
  * При вызове консольных приложений на экране могли мелькать цветные артефакты (при использовании расширения шрифтов/цветов).
  * Far3 build 2078.


## Build 110616 ##
#### Bugfixes ####
  * [Issue 403](https://code.google.com/p/conemu-maximus5/issues/detail?id=403), [Issue 404](https://code.google.com/p/conemu-maximus5/issues/detail?id=404): При 'Far /w' в фар не передавались нажатия мышки и некоторые клавиатурные сочетания (CtrlUp, CtrlDown, и т.п.).
  * [Issue 402](https://code.google.com/p/conemu-maximus5/issues/detail?id=402): После окончания выполнения консольной команды в некоторых случаях некорректно отрисовывались панели.


## Build 110610 ##
#### Bugfixes ####
  * [Issue 374](https://code.google.com/p/conemu-maximus5/issues/detail?id=374): при запуске DOS-приложений (ntvdm.exe) выскакивает MessageBox.
  * В некоторых случаях при запуске DOS-приложений (ntvdm.exe) схлопывался размер окна ConEmu.
  * В некоторых случаях при выходе из Far или запуске консольных команд ConEmu.exe мог аварийно завершаться.
  * [Issue 382](https://code.google.com/p/conemu-maximus5/issues/detail?id=382): в поле 'Another user' диалога создания новой консоли нельзя было ввести имя в формате 'domain\user'. В поле разрешена горизонтальная прокрутка.
  * В некоторых случаях окно ConEmu не закрывалось после завершения консольных процессов.
  * [Issue 384](https://code.google.com/p/conemu-maximus5/issues/detail?id=384): Не отключался флажок 'Install keyboard hooks'.
  * [Issue 400](https://code.google.com/p/conemu-maximus5/issues/detail?id=400): Правки в запускаторе.
#### Changes ####
  * Far3 build 2027.
  * Drop в любую вкладку, а не только в Far, например в cmd.exe. При Drop в не-Far-консоль вставляются полные имена перетаскиваемых файлов.
  * Запуск консоли с флагом SEE\_MASK\_NOASYNC.
  * Пункт 'Terminate' меню таба и кнопка 'Terminate' (диалог пересоздания консоли Win-W) игнорируют флажок 'Safe Far close'.


## Build 110308 ##
#### Bugfixes ####
  * [Issue 297](https://code.google.com/p/conemu-maximus5/issues/detail?id=297): Desktop mode и несколько мониторов.
#### Changes ####
  * [Issue 332](https://code.google.com/p/conemu-maximus5/issues/detail?id=332): Fixed/Cascade и подобные настройки не запрещаются в Maximized.
  * Плагины готовы к Far3 build 1900.
  * Добавлен msi для x64. Набор файлов тот же что и в msi x86, но только x64 версия может устанавливаться в 64битный 'Program Files'.
  * GuiMacro. Новая функция: FontSetName("FontFaceName"[,FontHeight[,FontWidth]]).

## Build 110306 ##
#### Bugfixes ####
  * Правки в запускаторе.
  * Не работал ввод через IME. Пока что ввод остается в отдельном плавающем окне, но результат попадает в консоль. Поскольку FAR игнорит нажатия с VK=0, иероглифы посылаются как VK=VK\_PROCESSKEY.
  * [Issue 373](https://code.google.com/p/conemu-maximus5/issues/detail?id=373): падение при запуске 'wmic'.
#### Changes ####
  * Экспериментально. Для планируемой поддержки возможности горизонтальной прокрутки ConEmu отрисовывает только видимую область [RealConsole](ConEmuTerms#RealConsole.md).
  * Разрешено сохранение опции 'Use ConEmuHk injects'. По умолчанию - включена.
  * Два новых файла в поставке (ConEmuCD.dll и ConEmuCD64.dll). В них вынесен код сервера консоли, чтобы разгрузить ConEmuHk.dll/ConEmuHk64.dll.
  * Интеграция с DosBox. Теперь можно запускать старые DOS-приложения в 64-битных ОС прямо из Far или батников.
    * DosBox брать здесь: http://www.dosbox.com (DirectLink: http://sourceforge.net/projects/dosbox/files/dosbox/0.74/DOSBox0.74-win32-installer.exe/download )
    * Ставить сюда: %ConEmuBaseDir%\DosBox
    * Создать (или переименовать) файл конфигурации: %ConEmuBaseDir%\DosBox\DosBox.conf
    * Дописать в конец файла команды монтирования дисков, смысл в том, что при запуске приложений в DosBox из ConEmu пути должны совпадать. Если на вашем NTFS диске отключены короткие имена Dos-приложение может и не запуститься. Для возможности запуска Dos-приложений из батников требуется включение флажка 'Use ConEmuHk injects'.
    * **Перезапустить** ConEmu, флажок 'DosBox (DOS apps)' должен включиться ([вкладка Features](Settings#Features.md)).
    * **Внимание**: Диск Z: в DosBox зарезервирован под эмулятор.
  * Через системное меню ConEmu можно было вызвать несколько экземпляров диалога свойств [RealConsole](ConEmuTerms#RealConsole.md). При закрытии одного из них валился conhost. /исправлено уже давно/
  * 'Extend foreground colors...' и 'Extend fonts' работают только для Far.


## Build 110227 ##
#### Changes ####
  * Documentation (http://code.google.com/p/conemu-maximus5/wiki/ConEmu) updated. Anybody read it?
  * Правки в запускаторе.
  * Добавлена опция 'Use ConEmuHk injects'. В release сборке пока отключена.
  * При включении лога 'Shell and processes' (вкладка Debug) отображаются LoadLibrary & FreeLibrary.


## Build 110225 ##
#### Bugfixes ####
  * Правки в запускаторе.
#### Changes ####
  * "ConEmuC.exe /GUIMACRO <macro command>". Возвращает errorlevel==0 в случае успеха, результат выводится в StdOut.
```
    Пример: "%ConEmuBaseDir%\ConEmuC.exe" /GUIMACRO WindowMinimize
```


## Build 110224 ##
#### Bugfixes ####
  * В 110223 поломалось отображение длинного вывода последней команды в Far1.
#### Changes ####
  * Немного макросов в папке Far3\_reg.


## Build 110223 ##
#### Changes ####
  * Настройка на вкладке 'Debug' - что журналировать. Закрытие окна настроек == выбор 'Disabled'.
  * Плагины готовы к Far3 build 1889.
  * По возможности при выполнении команд пропускаем cmd.exe (например, при выполнении команды 'netsh').


## Build 110218 ##
#### Major changes ####
  * Хуки системных функций теперь устанавливаются не через плагин, а через ConEmuHk.dll (что позволит обрабатывать не только far.exe, но и любой процесс, запущенный в этой консоли). В результате вылечен [Issue 60](https://code.google.com/p/conemu-maximus5/issues/detail?id=60), на подходе [Issue 65](https://code.google.com/p/conemu-maximus5/issues/detail?id=65) и портабельный реестр. Часть кода по «внедрению» взята из Console2.
  * Переменная окружения "ComSpec" более не переопределяется, а переменная "ComSpecC" теперь не создается.
  * Настройки конфигурации без имени (нет ключа "/config «name»" в параметрах ConEmu.exe) хранятся в "Software\ConEmu\.Vanilla". Если ключ "Software\ConEmu\.Vanilla" ранее отсутствовал - он создается копированием из "Software\ConEmu".
  * В тестовом режиме - поддержка Far3.
  * Изменения записи событий в консольный буфер ввода (ждем чистого буфера только для мышиных кнопок).
  * В диалог настроек добавлена вкладка 'Debug'. Если она открыта - в ней отображается журнал запускаемых в консоли процессов.
#### Bugfixes ####
  * [Issue 60](https://code.google.com/p/conemu-maximus5/issues/detail?id=60): зависание chcp на некоторых системах.
  * [Issue 364](https://code.google.com/p/conemu-maximus5/issues/detail?id=364): Visual Studio post build events.
  * При отключенной "[ ] Show column titles" не стартовал D&D.
  * Panel Views. Не ловились изменения цвета последнего элемента панели.
#### Changes ####
  * Отказ от WinEvents в сервере (ConEmuC.exe).
  * На вкладке Info показывает размер и видимость курсора.
  * Некоторые изменения умолчательных настроек.
    * Включено выделение мышкой (LShift - Text, LAlt - Block). RClick в BufferMode выполняет Paste.
    * Darkening = 255, Replace color indexes = '#1'.
  * Если на экране есть выделение (Text/Block) то RClick всегда делает Copy. (Не имеет отношения к Far's Grabber - AltIns).
  * Реструктуризация макросов Far в поставке (папки Far1\_reg, Far2\_reg, Far2\_fml).
  * Добавлен макрос для Far1 - изменение размера шрифта в ConEmu колесом мышки.
  * Для включения портабельности настроек **ConEmu** достаточно создать _пустой_ файл 'ConEmu.xml' рядом с ConEmuC.exe или с ConEmu.exe.
  * В меню по стрелке у кнопки '+' на toolbar отображаются укороченные строки запуска, чтобы меню не было слишком широким. При наведении курсора мышки на укороченный пункт меню - всплывает подсказка с полной строкой запуска.
  * [Issue 288](https://code.google.com/p/conemu-maximus5/issues/detail?id=288): В меню окна (правый клик по заголовку окна) добавлен пункт 'New console...', при удерживании Shift отображается диалог подтверждения, даже если флажок 'Create confirmation' отключен.
  * Проверка значения "FullScreen" в ключе [HKEY\_CURRENT\_USER\!Console] для Vista и выше.


## Build 110126 ##
#### Changes ####
  * По умолчанию hotkey 'Win-Del' не устанавливается. Включается через реестр или xml-файл.
  * GUI-макросы.
```
    FontSetSize(Relative,N)
     - изменить размер шрифта в ConEmu
       Relative==0: N - желаемая высота шрифта
       Relative==1: N (+-1, +-2) - увеличить/уменьшить шрифт
       Возвращает - "OK" или "InvalidArg"
    IsRealVisible
     - Проверка, видима ли RealConsole, "Yes"/"No"
    IsConsoleActive
     - Проверка, активна ли RealConsole, "Yes"/"No"
```
  * В реестре или xml можно настроить "ConInMode" - флажки 'QuickEdit Mode'/'Insert mode' в свойствах [RealConsole](ConEmuTerms#RealConsole.md) при старте.


## Build 110124 ##
#### Changes ####
  * Откат 'far /w' по умолчанию.
  * Настраиваемый hotkey (Win-X) для запуска новой консоли "cmd.exe".
  * Настраиваемый hotkey (Win-Del) для закрытия текущей консоли без подтверждения.
  * Опция ("Multi.AutoCreate") Win-Number создает новую консоль, если номер свободен.
  * Опция ("Multi.Iterate") Win-Number перебирает табы активной консоли.
  * Опция ("Multi.LeaveOnClose") не закрывать окно ConEmu после закрытия всех консолей.


## Build 110123 ##
#### Bugfixes ####
  * В x86 версии не запускались приложения из C:\Windows\Sysnative.
  * 'Text selection' не копировал текст, если начало выделения было правее/выше конца выделения.
  * В последних версиях Far 2 (при запуске без ключа /w) отвалилось включение длинного вывода программ.
#### Changes ####
  * В архивы (7z) предыдущих версий случайно попал ConEmu64.exe. Для сильно желающих, 64битная версия GUI доступна в msi.
  * При запуске ConEmu без параметров Far 2 по умолчанию запускается с ключом /w.
  * Положено начало 'GUI-макросам'. Смысл в том, чтобы дать фару 'порулить' параметрами ConEmu.
> > Пример вызова через callplugin (Far 2):
```
$if (callplugin(0x43454D55,"IsConEmu()")!=1 && env("ConEmuMacroResult")=="Yes") MsgBox("Macro", "Far started under ConEmu") $end
```
> > В Far 1 (врочем, как и в Far 2) для исполнения GUI-макроса (и просмотра его результата) можно использвать меню плагина.
> > Пока что поддерживаются следующие функции:
```
    IsConEmu()
     - возвращает "Yes" если Far прицеплен к ConEmu
    FindEditor("<FullEditFileName>"), FindViewer("<FullViewerFileName>"), FindFarWindow(<WindowType>,"<WindowTitle>")
     - возвращает "Found:<номер консоли>", "Blocked", или "NotFound"
    WindowMinimize() или WindowMinimize(1)
     - свернуть окно (если "1" - то в TSA)
    MsgBox("<Text>","<Title>","<ButtonType>")
     - показать GUI MessageBox, вернуть нажатую кнопку ("Ok", "Cancel", "Retry", и т.п.), ButtonType из Windows SDK.
```


## Build 110118 ##
#### Bugfixes ####
  * В меню по стрелке у кнопки '+' на toolbar первый пункт мог быть пустым.
#### Changes ####
  * Удерживая Shift при выборе пункта меню (стрелка у кнопки '+') открываем диалог параметров новой консоли
  * [Issue 355](https://code.google.com/p/conemu-maximus5/issues/detail?id=355): Что-то вроде общего пространства редакторов/вьюверов. Пример в 'ConEmu.Addons\ConEmu.Editor.fml'.


## Build 110117 ##
#### Bugfixes ####
  * [Issue 347](https://code.google.com/p/conemu-maximus5/issues/detail?id=347): Странность показа данных на закладках.
  * Не выполнялся батник: ver|find "6."
#### Changes ####
  * 'Press Enter to close console' -> 'Press Enter or Esc to close console'.
  * Стрелка у кнопки '+' на toolbar показывает меню из последних 16 запущенных команд, для быстрого запуска новой консоли.


## Build 110115 ##
#### Bugfixes ####
  * [Issue 350](https://code.google.com/p/conemu-maximus5/issues/detail?id=350): Click right button on close icon don't work while caption is hidden.
  * [Issue 351](https://code.google.com/p/conemu-maximus5/issues/detail?id=351): В Windows XP после обновления Far до версии 2.0 build 1783 перестал работать вывод консольных команд.
  * [Issue 353](https://code.google.com/p/conemu-maximus5/issues/detail?id=353): Не работал Detach в Far (CtrlAltTab).
  * [Issue 354](https://code.google.com/p/conemu-maximus5/issues/detail?id=354): "mycmd ." точка не передается команде.
#### Changes ####
  * [Issue 349](https://code.google.com/p/conemu-maximus5/issues/detail?id=349): При желании, в 64-битных ОС можно использовать 64-битный ConEmuC64.
  * Тестируем первую версию msi-инсталлятора.


## Build 110103 ##
#### Bugfixes ####
  * [Issue 346](https://code.google.com/p/conemu-maximus5/issues/detail?id=346): не работает запуск с комстроки.


## Build 101230 ##
#### Changes ####
  * Небольшие правки.


## Build 101225 ##
#### Bugfixes ####
  * [Issue 345](https://code.google.com/p/conemu-maximus5/issues/detail?id=345): Падение ConEmu.exe в некоторых случаях при просмотре файлов, содержащих '&' в имени папки/файла.


## Build 101224 ##
#### Bugfixes ####
  * [Issue 340](https://code.google.com/p/conemu-maximus5/issues/detail?id=340): При включенном Screen Gadgets (с настройками плагина по умолчанию) не работали RClick, D&D, и пр.
  * [Issue 343](https://code.google.com/p/conemu-maximus5/issues/detail?id=343): Блокировались табы после ошибки поиска в редакторе.
#### Changes ####
  * 3-е состояние флажка 'Always show scrollbar'.


## Build 101217 ##
#### Changes ####
  * Меню для таба.
  * Некоторые изменения background-api.


## Build 101215 ##
#### Bugfixes ####
  * ConEmu.exe мог падать при работе background-плагинов.
  * Attach из плагина не работал, если ConEmu.exe не лежал рядом с ConEmuC.exe.


## Build 101214 ##
#### Bugfixes ####
  * Can't open console data file mapping. ConEmuFarMapping.xxxx
#### Changes ####
  * Не работал аттач из консоли, запущенной под администратором.
  * Usability of buffer scrollbar. Прокрутка автоматически всплывает при изменении позиции в консоли. При слежении за курсором мышки всплытие и скрытие производится с задержкой.


## Build 101212 ##
#### Bugfixes ####
  * В Win2k даже при наличии Far-плагина при закрытии консоли возникало 'Root process was alive less than 10 sec'.
  * Правки /ATTACH.
  * В некоторых случаях, при включении прокрутки (Long console output), консоль прокручивалась до самого низа.
  * Drag-n-Drop: При дропе из некоторых источников копировался только первый файл.
#### Changes ####
  * Drag-n-Drop: Поддержка дропа из 'устройств' папок и в папки с длинными путями.
  * При сильной загрузке процессора ConEmu мог завершиться на этапе старта после появления в консоли сообщения 'Process was not attached to console. Is it GUI?'.
  * Для удобства аттача свободных консолей в поставку включен файл 'Attach.cmd'.
  * Уточнения API для 'background' плагинов. Обновлен 'Panel colorer' (исправленные исходники доступны на svn).


## Build 101207 ##
#### Bugfixes ####
  * [Issue 334](https://code.google.com/p/conemu-maximus5/issues/detail?id=334): Правки 'Attach to ConEmu' из плагина.
#### Changes ####
  * [Issue 318](https://code.google.com/p/conemu-maximus5/issues/detail?id=318): Расширена настройка действий правой и средней кнопки мышки для работы с буфером обмена. Например, на правую кнопку мышки можно назначить Paste для 'режима с прокруткой' (cmd), или всегда, при нажатом LShift. Напомню, что 'far /w' не считается 'режимом с прокруткой'.
  * [Issue 283](https://code.google.com/p/conemu-maximus5/issues/detail?id=283): 'Always show scrollbar' option.
  * [Issue 333](https://code.google.com/p/conemu-maximus5/issues/detail?id=333): При отключенной 'Always show scrollbar' полоса всплывает при наведении мышки только если НЕ нажаты Ctrl или Alt или Shift или левая кнопка мышки.


## Build 101129 ##
#### Bugfixes ####
  * [Issue 327](https://code.google.com/p/conemu-maximus5/issues/detail?id=327): 'Syncho events are pending!' по F10.
  * [Issue 324](https://code.google.com/p/conemu-maximus5/issues/detail?id=324): 'mh\_ConEmuCInput not found'.


## Build 101128 ##
#### Bugfixes ####
  * Far2 падал при попытке выгрузки плагина (http://bugs.farmanager.com/view.php?id=1604).
#### Changes ####
  * 'Tray' переименован в 'TSA' (Taskbar Status Area).
  * Добавлен флажок 'Always show TSA icon'.


## Build 101125 ##
#### Bugfixes ####
  * [Issue 298](https://code.google.com/p/conemu-maximus5/issues/detail?id=298): HG & Araxis (detached comspec).
  * [Issue 321](https://code.google.com/p/conemu-maximus5/issues/detail?id=321): PanelView, некорректный скроллинг при выделении элементов панели.
  * [Issue 322](https://code.google.com/p/conemu-maximus5/issues/detail?id=322): Если Far был запущен без ConEmu.exe, то при наличии плагина ConEmu.dll происходило падение при открытии редактора/вьювера.


## Build 101120 ##
#### Bugfixes ####
  * PanelView. Не работал в 'far /w'.
  * [Issue 317](https://code.google.com/p/conemu-maximus5/issues/detail?id=317), [Issue 308](https://code.google.com/p/conemu-maximus5/issues/detail?id=308): В некоторых случаях терялся PID процесса FAR с плагином ConEmu.
  * При сильной загрузке процессора в Win7 нажатие Win-Number могло проваливаться в Explorer.
  * [Issue 316](https://code.google.com/p/conemu-maximus5/issues/detail?id=316): Ненужное сообщение об ошибке записи в xml, при отсутствии прав на изменение файла.
  * Таб не переключался в другую консоль, если активируемое окно фара заблокировано диалогом.
#### Changes ####
  * Новая реструктуризация.
    * 64-битные версии ConEmu.exe & ConEmuC.exe более не поставляются за ненадобностью.
    * ConEmu.exe помещен в корень дистрибутива, ConEmuC.exe и прочие файлы в папку ConEmu.
    * При желании (чтобы не 'захламлять' папку фара) ConEmu.exe можно также переместить в папку ConEmu.
    * Плагины для Far как и ранее лежат в папке 'Plugins'.
    * Файл ConEmu.xml и шрифты могут лежать как рядом с ConEmu.exe, так и в папке ConEmu (рядом с ConEmuC.exe).
    * Старые файлы рекомендуется удалить самостоятельно.
  * Переменная окружения 'ConEmuBaseDir', содержит папку с файлами ConEmuC.exe, ConEmuHk.dll, и т.п.


## Build 101108 ##
#### Bugfixes ####
  * [Issue 308](https://code.google.com/p/conemu-maximus5/issues/detail?id=308): В некоторых случаях терялся PID процесса FAR с плагином ConEmu. В результате мог отваливаться D&D и прочее.
  * [Issue 312](https://code.google.com/p/conemu-maximus5/issues/detail?id=312): Мусор по краям консоли в Maximized & FullScreen при включенном центрировании.
  * [Issue 313](https://code.google.com/p/conemu-maximus5/issues/detail?id=313): Global hotkey may be used for minimize/restore of ConEmu window (Win-C by default). Кнопку ('C' по умолчанию) можно настроить через параметр 'MinimizeRestore', модификатор соответствует HostKey (кроме Apps и без различия правый/левый). Глобальный хоткей работает только в первом запущенном экземпляре ConEmu, и только если он уже не зарегистрирован для какого-либо другого приложения. После закрытия первого запущенного экземпляра ConEmu хоткей 'наследуется' другим экземпляром.
  * Авторегистрация шрифтов выполняется не только для `*`.ttf файлов в папке программы, но и в текущей (current directory).
  * В 'ConEmu Underlines' добавлен флажок 'Hilight plugin panels'. (Плагин предназначен для демонстрации возможностей API)
  * В корне дистрибутива помещен небольшой 'starter' с именем 'ConEmu.exe'. Предназначен для более удобного запуска Far Manager-а в ConEmu. Этот файлик просто выполняет запуск ConEmu\ConEmu.exe с теми же аргументами.
  * 'RighClick 4 context menu'. При третьем состоянии, в течении 'длинного правого клика' отображается Throbber вокруг курсора. Когда кружок замыкается - после отпускания правой кнопки мышки будет показано EMenu.
#### Changes ####
  * Telnet detector changed. Теперь переменная 'TERM' игнорируется.
  * Blues16.bmp & Greys16.bmp from ccaidd.


## Build 101101 ##
#### Bugfixes ####
  * Исправлены некоторые баги перезапуска консоли.
#### Changes ####
  * PanelView. Асинхронная загрузка иконок и превьюшек.


## Build 101022 ##
#### Bugfixes ####
  * [Issue 311](https://code.google.com/p/conemu-maximus5/issues/detail?id=311): Падение Far 1.7x при переключении табов.
  * Правки 'ConEmu Underlines'. Включение/отключение и смена цвета налету, без закрытия диалога настройки.


## Build 101021 ##
#### Bugfixes ####
  * [Issue 306](https://code.google.com/p/conemu-maximus5/issues/detail?id=306): Не работало переключение раскладок Ильи Бирмана.
#### Changes ####
  * Системное меню всплывает по Win-Alt-Space.
  * Перейти в 'полноэкранный' режим можно также по Ctrl-Win-Enter.
  * Win-Alt-P открывает диалог настроек ConEmu.
  * В поставку добавлен плагин 'ConEmu Underlines'. Смысл - разлиновать рабочую область панелей Far Manger. Плагин можно включить через F11 (или AltShiftF9 в панелях) в FAR и выбрать желаемый цвет линий. В настройках GUI должен быть включен флажок 'Allow Plugins' в группе 'Background Image'. Активация фичи отключает отображение файла 'Background Image'. Если вместо ожидаемой картинки отображается черный фон - вероятно движок 'Darkening' установлен в 0. Передвиньте движок вправо.


## Build 101019 ##
#### Bugfixes ####
  * [Issue 299](https://code.google.com/p/conemu-maximus5/issues/detail?id=299): При включении '/log' ConEmuC падал при выходе.
#### Changes ####
  * Добавлен флажок "Send Alt+Space to console".
  * Доработки Background API.


## Build 101017 ##
#### Warning ####
  * Реструктуризация поставки ConEmu. Исполняемые файлы, WhatsNew, Addons, Settings и пр. перемещены в подпапку "ConEmu". Сделано для удобства распаковки в папку Far Manager (чтобы не мешать файлы). Удалите старые файлы ConEmu!
  * Обе версии ConEmu (x86 & x64) теперь поствляются в одном архиве (добавлены файлы ConEmu64.exe, ConEmuC64.exe, ConEmuHk64.dll).
#### Bugfixes ####
  * WinXP. Консоли не переключались по RWin-Number.
  * Ошибочное отображения в табе заголовка соседней консоли во время создания новой консоли.
  * PanelView. При попытке запуска плагина возникала ошибка "-1" в Far x86 & ConEmu x64.
  * [Issue 218](https://code.google.com/p/conemu-maximus5/issues/detail?id=218): Побеждено мелькание кейбара при ресайзе окна.
  * Не всегда правильно работало переключение раскладок и слежение за раскладкой в консоли (Win7x64).
#### Changes ####
  * PanelView. Изменена реакция на PgUp/PgDn в режиме Tiles.
  * int WINAPI SyncExecute(SyncExecuteCallback\_t CallBack, LONG\_PTR lParam);
  * При запуске без параметров ConEmu пытается искать FAR: a) в папке с ConEmu.exe; b) в текущей директории; c) на уровень вверх от ConEmu.exe. Если Far.exe не найден - запускается cmd.
  * Если другие плагины Far экспортируют функцию "void WINAPI OnConEmuLoaded(struct ConEmuLoadedArg`*` pConEmuInfo);" то ConEmu.dll вызовет ее после инициализации плагина ConEmu или после аттача к ConEmu GUI.


## Build 100919a ##
#### Bugfixes ####
  * При минимизированном conemu не пропадал зеленый градусник на панели задач.
  * Не обновлялся заголовок таба вьювера при смене файла через Gray+/Gray-.
  * CtrlBreak не прерывал 'зависшие' макросы в Far.
  * В некоторых случаях на Windows XP при запуске могла мелькать [RealConsole](ConEmuTerms#RealConsole.md).
  * [Issue 274](https://code.google.com/p/conemu-maximus5/issues/detail?id=274): Окно [RealConsole](ConEmuTerms#RealConsole.md) больше не двигается в процессе работы. Флажок 'Lock real console pos' убран за ненадобностью.
  * Для корректного отображения !EMenu (во всех его режимах) перехватываются функции GetWindowRect и ScreenToClient.
  * [Issue 280](https://code.google.com/p/conemu-maximus5/issues/detail?id=280): Навигация в режиме тумбнейлов.
#### Changes ####
  * PanelViews теперь доступен и в Far1.


## Build 100906 ##
#### Bugfixes ####
  * Фон, заданный плагином отображался не только в Far.


## Build 100904 ##
#### Bugfixes ####
  * [Issue 290](https://code.google.com/p/conemu-maximus5/issues/detail?id=290): Exception triggered in !CRealConsole::MonitorThread.
#### Changes ####
  * **Background image**: картинка затемняется в соответсвии с "Fade contents when inactive".
  * **Background image**: флажок 'Allow plugins' в третьем состоянии разрешает отрисовку фона плагинами только в панелях.


## Build 100903 ##
#### Bugfixes ####
  * В консоли не детектился 'одноциферный' прогресс операции.
  * При отключенном 'Fix FAR borders' не всегда срабатывал 'Enhance progressbars and scrollbars'.
#### Changes ####
  * **API**: CECMD\_SETBACKGROUND.
  * **Background image**: автоматическое перечитывание измененной картинки.
  * **Background image**: режимы Stretch & Tile. Для примера использования 'Tile' добавлен файл 'ConEmu.Addons\Lines`*`.bmp' для высоты шрифта 16 и 18. Для 'Lines`*`.bmp' рекомендуется оставить в 'Replace color indexes' только цвет панелей ('#1' в стандартной цветовой схеме фара).
  * **Background image**: в окне настройки разрешено редактирование поля 'Replace color indexes'.
  * Обработка невозможности создания шрифтов.
  * **Enhance progressbars**: В некоторых случаях на "темных" мониторах 25% серый не отличался от черного. Добавлен параметр "PartBrushBlack", правка только через реестр/xml.


## Build 100814 ##
#### Bugfixes ####
  * Закрытие фара макросом (SafeFarClose) выполняется только если фар подает признаки жизни.
  * [Issue 281](https://code.google.com/p/conemu-maximus5/issues/detail?id=281): затычка для PgUp/PgDn.


## Build 100804 ##
#### Bugfixes ####
  * Поправлено движение мышки при нажатых кнопках (надеюсь изничтожено неожиданное закрытие диалогов по !RBtnUp).
  * Far начинал реагировать на 'No zone check' только после перещелкивания флажка.
#### Changes ####
  * Достало виндовое окошко 'End program', поэтому все-таки добавлен флажок 'Safe Far Close'. По умолчанию включен. Если включить - при закрытии ConEmu крестиком в плагин Far'а посылается макрос:
```
    @$while (Dialog||Editor||Viewer||Menu||Disks||MainMenu||UserMenu||Other||Help)
      $if (Editor) ShiftF10 $else Esc $end
    $end
    Esc
    $if (Shell) F10 $if (Dialog) Enter $end $Exit $end
    F10
```
> > Макрос можно настроить (реестр/xml), параметр "SafeFarCloseMacro".
> > Внимание! Если перед закрытием окна в Far открыты редакторы - они могут быть сохранены в режиме сохранения изменений.
  * Некоторые изменения в установке хуков (плагин).


## Build 100704 ##
#### Bugfixes ####
  * [Issue 29](https://code.google.com/p/conemu-maximus5/issues/detail?id=29): Сбивался ClearType при переключении режима дисплея.
  * [Issue 275](https://code.google.com/p/conemu-maximus5/issues/detail?id=275): Любителям консоли шириной 341 символ.
#### Changes ####
  * Добавлена визуальная настройка шрифта для [RealConsole](ConEmuTerms#RealConsole.md). Запускается с вкладки 'Features', кнопочка рядом с 'Show real console'.
  * Перед запуском консоли ConEmu проверяет корректность шрифта указанного в настройке для [RealConsole](ConEmuTerms#RealConsole.md). Если шрифт не установлен в системе, не является юникодным (.ttf), или не зарегистрирован в реестре - отображается диалог настройки шрифта.
  * Добавлен флажок 'Lock real console pos'. При его включении невидимая реальная коноль не двигается (не 'убегает' от курсора мыши).
  * В ConEmu Panel Views добавлен самостоятельный модуль ico (иконки в gdi+ больше не отдаются).


## Build 100627 ##
#### Bugfixes ####
  * Продолжение 'far /w'. Некорректный ресайз окна при переключении вкладок.
  * [Issue 211](https://code.google.com/p/conemu-maximus5/issues/detail?id=211): Уменьшена прожорливость неактивных консолей.
  * [Issue 264](https://code.google.com/p/conemu-maximus5/issues/detail?id=264): переключение между вкладками после AltF9 / AltEnter.
  * [Issue 266](https://code.google.com/p/conemu-maximus5/issues/detail?id=266): AutoTabs при /w.
  * [Issue 267](https://code.google.com/p/conemu-maximus5/issues/detail?id=267): Сбивалось разрешение после возвращения из досовского приложения.
  * [Issue 268](https://code.google.com/p/conemu-maximus5/issues/detail?id=268): по окончании копирования (и т.п.) оставался красный прогресс на таскбаре.
  * [Issue 269](https://code.google.com/p/conemu-maximus5/issues/detail?id=269): Игнорировался ключ /K: '%comspec% /K'.
#### Changes ####
  * В 'PanelViews' в режиме Tiles теперь для PE файлов (.exe, .dll, и пр.) отображается версия, битность и флаги Far1/Far2, .Net и Upx.
  * Добавлена новая настройка - флажок 'No zone check', по умолчанию отключен. Влияет на выполнение фаром .exe файлов. Same as appeared in Far 2 build 771 and disappeared after 1464.
  * Автоскрытие полосы прокрутки при уезжании курсора мышки за пределы окна.


## Build 100611 ##
#### Bugfixes ####
  * [Issue 257](https://code.google.com/p/conemu-maximus5/issues/detail?id=257): Background Image при 3м состоянии опции Monospace.
  * Не всегда корректно определялось наличие PanelTabs.
  * [Issue 254](https://code.google.com/p/conemu-maximus5/issues/detail?id=254): Fix Alt on AltTab теперь и для AltF9.
  * 'Can't send console event (pipe)' больше не показывается.
  * [Issue 261](https://code.google.com/p/conemu-maximus5/issues/detail?id=261): Заглушка при выполнении HttpSendRequest.
  * Дополнительное логирование RClick (ConEmu.exe /log).


## Build 100610 ##
#### Bugfixes ####
  * Падение при закрытии Far x64 при загруженном ConEmuTh.dll.
  * Еще немного 'far /w': контекстное меню по правой кнопке, драг правой кнопкой.
  * Плагин ConEmu.dll в списке плагинов теперь отображается просто как 'ConEmu'. Макросы для Far 1.7x переделаны на 'menu.select'.
  * [Issue 261](https://code.google.com/p/conemu-maximus5/issues/detail?id=261): в поиске...


## Build 100609 ##
#### Bugfixes ####
  * При запуске консольных приложений по ShiftEnter (-new\_console) не включался режим буфера.
  * Не устанавливалась ConEmuArgs.
  * После показа не пряталась админская консоль в Win7.
  * Убран еще один пережиток ConsoleCharWidth.
  * При выполнении консольных команд дублировался !CtrlC (^C).
  * Can't send console event (pipe).
  * [Issue 259](https://code.google.com/p/conemu-maximus5/issues/detail?id=259): Не происходила отрисовка при отключенных табах.


## Build 100608 ##
#### Bugfixes ####
  * Поломался драг при постоянном верхнем меню.
  * Не работали PanelViews при постоянном верхнем меню.


## Build 100606 ##
#### Bugfixes ####
  * [Issue 246](https://code.google.com/p/conemu-maximus5/issues/detail?id=246): Если не удалось установить флаг 'AlwaysOnTop' для [RealConsole](ConEmuTerms#RealConsole.md) - после CtrlWinAltSpace фокус не будет возвращаться в ConEmu.
  * При закрытии модального редактора/вьювера не обновлялись табы.
  * [Issue 244](https://code.google.com/p/conemu-maximus5/issues/detail?id=244): В некоторых случаях ошибочно разрешался переход на другие табы.
  * [Issue 249](https://code.google.com/p/conemu-maximus5/issues/detail?id=249): Унификация переменных окружения (ConEmuDir, ConEmuArgs, ConEmuHWND).
  * Mouse cursor shape.
  * Активация редактора/вьювера при наличии QSearch в панелях.
  * Упрощен способ получения списка окон (табы).


## Build 100531 ##
#### Bugfixes ####
  * Во время множественного запуска консольных программ (i.e. компиляция проекта) ConEmu блокировался.
  * Изменен алгоритм обновления табов Tabs.
  * После смены окна средствами Фар - слетел стек последних открытых.


## Build 100530 ##
#### Bugfixes ####
  * Отвалилась установка BufferHeight на время выполнения из Far (без /w) консольных команд.
  * В некоторых случаях колесо мышки не проходило в консоль.
  * Поправлен 'Hourglass if not responding'.
  * После 'Jump to' в 'Multimedia Viewer' переставала работать клавиатура.
#### Changes ####
  * Option 'Save window size and position on exit'.


## Build 100526 ##
#### Bugfixes ####
  * Измененный список процессов консоли не приходил в GUI до любого изменения содержимого консоли.
  * Keyboard fixes for dead charachers.
#### Changes ####
  * Доработки PanelViews.
  * [Issue 235](https://code.google.com/p/conemu-maximus5/issues/detail?id=235): Drag&Drop. Разрешено создание ярлыков при драге более одного файла. Side effect: теперь можно дропать в FooBar2000 default interface.
  * Поддержка Far2 /w.


## Build 100520 ##
#### Bugfixes ####
  * Minor D&D bugfixes.
  * Не работал 'Edit/View console output'.
#### Changes ####
  * PanelViews. Mouse is working now. Small restriction: when clicked item is not visible on real panel - plugin executes macro "panel.setposidx(...)" instead of left or right click.


## Build 100518 ##
#### Bugfixes ####
  * Поломалась палитра для 'Fade contents…'.
  * Drawing performance issues.
#### Changes ####
  * Изменен способ передачи содержимого консоли в GUI - опять пайпы. FileMapping с информацией о консоли (CESERVER\_REQ\_CONINFO\_HDR) создается.
  * Поддержка новых макросных индикаторов FAR2.
  * Некоторое улучшение обработки закрытия консоли в режиме 'Press Enter to close console…'.
  * В диалоге плагина 'Unicode CharMap' блок символов отображается соответствующим шрифтом. Опция 'Extend Unicode CharMap' включена по умолчанию.
  * Доработки PanelViews. Автоповорот превьюшек по EXIF. Подсказки в настройках.

## Build 100505 ##
#### Bugfixes ####
  * [Issue 197](https://code.google.com/p/conemu-maximus5/issues/detail?id=197): Неясное поведение HideCaption.
  * [Issue 229](https://code.google.com/p/conemu-maximus5/issues/detail?id=229): Не переключается опция 'Block inactive cursor'.
  * Доработки PanelViews.


## Build 100429 ##
#### Bugfixes ####
  * Не скрывался PictureView на Win7, при переходе по вкладкам, когда консоль и ConEmu были запущены под разными пользователями.
  * В некоторых случаях при запуске новой консоли под администратором в Win7 возникала ошибка: Can't open ConEmuC process! Attach is impossible!
#### Changes ####
  * В окне 'About' показывается битность приложения.
  * Доработки PanelViews.


## Build 100428 ##
#### Bugfixes ####
  * В некоторых случаях после запуска 16бит приложений ConEmu не закрывался (из-за процесса ntvdm.exe).
#### Changes ####
  * Во избежание некоторых казусов - мышиные события не посылаются в консоль, когда в ней есть прокрутка.
  * RightClick на табе при удерживаемом Control = MiddleClick: сразу выполняется Close для фара или поднимается диалог пересоздания консоли.
  * Опция 'Sleep in background' - снижать частоту опроса и отрисовки когда ConEmu не в фокусе.
  * [Issue 217](https://code.google.com/p/conemu-maximus5/issues/detail?id=217): В настройке в выпадающих списках моноширные шрифты подсвечиваются жирным.
  * Мышиный курсор над клиентской частью окна меняется только когда окно ConEmu в фокусе.
  * Во время работы Telnet.exe игнорируется размер консольного курсора.
  * Имя параметра 'ConsoleCharWidth' заменено на 'ConsoleFontWidth' (как и было ранее указано в reg файле).
  * Для удобства, имя пользователя и пароль теперь можно ввести в окне подтверждения создания новой консоли. Флажок 'Run as' (или 'Run as administrator' в Vista и выше) по прежнему работает самостоятельно.
  * Добавлена настройка позволяющая отключить флэшинг иконки на панели задач и рамки окна (для фара и для остальных приложений раздельно). Третье состояние флажков - мигать только иконкой, не трогая рамку окна.
  * Thumbnails and Tiles - FAR2 only. Плагин, позволяющий в Far Manager менять вид панелей - отображение иконок, превьюшек, и прочего. В качестве бонуса - просмотр 'на лету' иконок и картинок в плагине ImpEx. Все размеры и цвета настраиваются на вкладке 'Views' настроек ConEmu.


## Build 100404 ##
#### Bugfixes ####
  * [Issue 197](https://code.google.com/p/conemu-maximus5/issues/detail?id=197): Некорректно включался 'Hide caption' в уже развернутом окне.
#### Changes ####
  * Обновлен макрос ShiftEnter для панелей.
  * D&D мог не работать в !WinPE.
  * Оптимизирована передача ввода в консоль (обработка пачек сообщений).


## Build 100326 ##
#### Bugfixes ####
  * [Issue 197](https://code.google.com/p/conemu-maximus5/issues/detail?id=197): Некорректно включался 'Hide caption' в уже развернутом окне.
#### Changes ####
  * Обновлен макрос ShiftEnter для панелей.
  * D&D мог не работать в !WinPE.
  * Оптимизирована передача ввода в консоль (обработка пачек сообщений).
  * Ready for Thumbnails.


## Build 100326 ##
#### Bugfixes ####
  * Не работало обратное переключение консолей (по умолчанию Win-Shift-Q).
#### Changes ####
  * Перетрях диалога настроек. Добавлена новая вкладка 'Tabs'. На ней теперь настройки хоткеев, мультиконсоли и табов.
  * «Host-key»+Number можно отключить.
  * Разрешен D&D на с активной на активную панель в режиме создания ярлыка.
  * !RClick на табе или кнопке с номером консоли на тулбаре вызывает контекстное меню.
  * В контекстном меню (таб/тулбар) созданы пункты 'Save' и 'Save all' для сохранения открытых редакторов FAR в **активной** консоли. Макрос для 'Save all' можно настроить, для 'Save' выполняется макрос 'F2'.
  * В контекстном меню (таб/тулбар) создан пункт 'Detach' - показать [RealConsole](ConEmuTerms#RealConsole.md) и отцепиться от нее (если это единственная консоль - ConEmu будет закрыт).


## Build 100324 ##
#### Bugfixes ####
  * Обновлен сертификат! В старом был неправильно указан 'ComEmu'.
  * В консоль не проходила комбинация AltShiftEnter.
  * D&D ошибочно активировался, если помимо указанного (или вообще отсутствующего) модификатора - удерживались и другие. Например для драга ЛКМ модификатор вообще не был указан, но драг начинался при удерживаемом Alt.
#### Changes ####
  * Изменен способ передачи ввода в консоль (теперь пересылается только через pipe).
  * В неактивных консолях снижается частота опроса.


## Build 100320 ##
#### Changes ####
  * ConEmuHk.dll (обработка Win-Number) загружается только в ОС Vista и выше. У пользователя однократно запрашивается разрешение на установку хука (дабы не было вопросов 'Почему всплыл антивирус').
  * Доработано выделение/вставка текста консоли.
    * Выделение теперь может быть 'блочным' (как в обычной консоли) или 'текстовым' (как в текстовых редакторах).
    * В меню окна добавлены пункты 'Mark block' и 'Mark text'.
    * Добавлена настройка клавиш мышки и модификаторов. Теперь можно указать когда разрешено выделение мышкой:
      * Только при наличии прокрутки (Buffer only, по умолчанию), или всегда (Always).
      * Можно указать модификатор (Alt, Shift, ...) для обоих типов выделения (Block & Text).
      * Можно выбрать действия для правой и средней кнопки мышки (Copy/Paste).
      * Можно настроить цвет фона и текста для выделения.
    * Снято ограничение на флажок 'Mouse' в FAR (его можно оставлять включенным).
    * **Note**. Paste выполняется средствами консоли.
    * **Note**. Копирование выполняется 'с экрана', то есть с выделением в редакторе FAR не имеет ничего общего.


## Build 100318 ##
#### Changes ####
  * [Issue 69](https://code.google.com/p/conemu-maximus5/issues/detail?id=69): Добавлена поддержка выделения/копирования текста мышкой (аналог консольного 'QuickEdit mode'). Если этим хочется пользоваться в FAR2 (AltIns по какой-то причине не устраивает) - необходимо отключить флажок 'Mouse' в настройке FAR 'Interface settings'.
  * На вкладке 'Info' появился новый счетчик.


## Build 100309 ##
#### Bugfixes ####
  * [Issue 206](https://code.google.com/p/conemu-maximus5/issues/detail?id=206): Win-Number вызывал Start menu, если Win отпускался перед Number.


## Wiki ##
  * Documentation was started as [Wiki](x64_or_x86.md). Comments and additions are accepted.


## Build 100308 ##
#### Changes ####
  * Обработка Win-Number в Windows7. При отсутствии ConEmuHk.dll эти комбинации обрабатываются ОС.


## Build 100306 ##
#### Bugfixes ####
  * Падение, если командный файл запуска консолей не содержал перевода строки.
  * [Issue 203](https://code.google.com/p/conemu-maximus5/issues/detail?id=203): Уточнения в 'Press Enter to close console'.
#### Changes ####
  * DblClick на правой/левой рамке окна растягивает окно по горизонтали, а на верхней/нижней - по вертикали.


## Build 100304 ##
#### Bugfixes ####
  * [Issue 198](https://code.google.com/p/conemu-maximus5/issues/detail?id=198): Redraw in FAR1.
  * [Issue 199](https://code.google.com/p/conemu-maximus5/issues/detail?id=199): Не отключался Fade-effect.
  * [Issue 201](https://code.google.com/p/conemu-maximus5/issues/detail?id=201), [Issue 202](https://code.google.com/p/conemu-maximus5/issues/detail?id=202): Ошибочно включался режим прокрутки после аттача консоли к ConEmu.
  * При включенном 'Transparent «User screen»' или отключенном 'Monospace' в некоторых случаях падали с переполнением стека.
  * Minor resize optimization.


## Build 100223 ##
#### Bugfixes ####
  * [Issue 196](https://code.google.com/p/conemu-maximus5/issues/detail?id=196): ConEmu вызывал ошибки отрисовки в FAR.
#### Changes ####
  * Форматированный вывод настроек в XML (для новых ключей и значений).


## Build 100222 ##
#### Bugfixes ####
  * При DblClick по заголовку окна устанавливался некорректный размер консоли.
  * [Issue 189](https://code.google.com/p/conemu-maximus5/issues/detail?id=189): На некоторых машинах после Detach в FAR (CtrlAltTab) происходила перезагрузка.
  * [Issue 190](https://code.google.com/p/conemu-maximus5/issues/detail?id=190): ConEmu20100213, выгрузка плагина ConEmu.
  * [Issue 195](https://code.google.com/p/conemu-maximus5/issues/detail?id=195): Fade effect и Download Master.
  * Убрана небольшая задержка при старте ConEmu.
  * После запуска `ConEmu.exe /bufferheight 0 /cmd ...` не работала кнопка перехода в буферный режим.
  * Контекстное меню не всплывало если правый клик приходил как !RDblClick (он всегда уходил в консоль как !RClick).
#### Changes ####
  * Флажок 'Auto' запрещен при выборе фиксированного размера растрового шрифта.
  * На вкладку 'Colors' добавлена настройка 'Block inactive cursor' (курсор в виде пустого прямоугольника, когда консоль неактивна).
  * Добавлена визуальная настройка Fade-effect (затенение содержимого консоли когда ConEmu не в фокусе). Теперь можно как затенять, так и осветлять содержимое консоли.
  * Добавлена задержка скрытия рамки в режиме 'Hide caption always'. В диалог настройки добавлена возможность задания ширины рамки и длительности задержек.
  * При ресайзе проверяем максимально допустимый размер real-консоли.
  * Немного ускорена передача клавиатурного и мышиного ввода в консоль.
  * Возможность настроить 'Host-клавишу' для горячих клавиш мультиконсоли. Ранее были жестко выбраны правый или левый Win. Теперь можно выбрать Win, Apps, и [правый|левый|любой] Shift/Ctrl/Alt. Можно выбрать комбинацию до 3-х модификаторов.  Включение в диалоге настройки 'Hotkey settings' обоих флажка модификатора (например LCtrl & RCtrl) означает 'любой'.  Не рекомендуется использовать Apps в сочетании с другими модификаторами - Apps провалится в консоль.
  * Доработана прозрачность «User screen». Место под приподнятыми панелями считается прозрачным. CtrlAltShift показывает «User screen» (временно отменяет прозрачность).
  * В поставку добавлен файл 'ConEmu.Addons\CallPlugin.txt'. В нем перечислены возможные коды команд callplugin (макросы FAR2).
  * Доработан ресайз панелей мышкой. В FAR2 можно перед началом ресайза высоты зажать Shift, тогда будет меняться высота правой или левой панели (той, по рамке которой щелкнули).
  * [Issue 160](https://code.google.com/p/conemu-maximus5/issues/detail?id=160): Разрешен драг элемента '..'. При этом перетаскивается текущая папка (не ее содержимое!). **Внимание**: допустимость такого драга НЕ проверяется и остается на совести пользователя.  При дропе на соседнюю панель предварительно выполняется макрос: `$If (APanel.SelCount==0 && APanel.Current=="..") CtrlPgUp $End`


## Build 100213 ##
#### Bugfixes ####
  * Поправлена работа в многомониторных конфигурациях (каскад и FullScreen).
  * [Issue 182](https://code.google.com/p/conemu-maximus5/issues/detail?id=182): При запуске в многомониторных конфигурациях стартует с урезанной шириной.
  * [Issue 183](https://code.google.com/p/conemu-maximus5/issues/detail?id=183): !EMenu появлялось только после движения мышки.
  * [Issue 185](https://code.google.com/p/conemu-maximus5/issues/detail?id=185): После максимизации окна в заголовоке окна отключаются темы.
  * [Issue 186](https://code.google.com/p/conemu-maximus5/issues/detail?id=186): Не работал Maximized режим запуска ConEmu при старте.
  * В некоторых случаях сразу после запуска уменьшался размер окна (ошибка установки мелкого шрифта в real-console).
#### Changes ####
  * Когда ConEmu не в фокусе - курсор (если он видим в консоли) отображается прямоугольником.
  * Fade-effect содержимого консоли когда ConEmu не в фокусе.
  * Доработано проявляение скрытой рамки с заголовком. Задержку можно настроить.
  * В Desktop режиме отключены Cascade & FullScreen.
  * Щелчок правой кнопкой по рамке больше не сворачивает в трей. Только щелчок по крестику.


## Build 100211 ##
#### Bugfixes ####
  * В 100208 поломался драг - всегда выполнялся Move вместо Copy.
#### Changes ####
  * При перетаскивании из корзины учитывается флажок 'Copy by default' (файл из корзины не удаляется).
  * Изменен способ активации плагина (теперь FAR1 & FAR2 работают одинаково).
  * 'Always on top' в настройке и системном меню.
  * Улучшена работа при наличии нескольких мониторов.
  * Переделано скрытие заголовка 'Hide caption always'. Теперь заголовок скрывается вместе с рамкой, но при наведении мышки на скрытый заголовок/рамку - они появляются. Вокруг видимой части остается видимой тонкая (1 пиксел) часть рамки. Ширина настраивается "HideCaptionAlwaysFrame".
  * Желающие иметь FAR на рабочем столе могут поиграться с новой версией. Включать флажок 'Desktop mode'. При желании - настроить стартовый размер и положение.
  * В тестовом режиме появилась 'прозрачность панелей'. Пока тормознуто и будет переделано. Смысл в том, чтобы при скрытии одной или двух панелей вместо них отображался рабочий стол, но сохранялась возможность посмотреть «User screen» приподняв край панелей (той же мышкой, например:). Включать флажок 'Transparent «User screen» while panel(s) is turned off'. По умолчанию установлен цвет 'ColorKey' - '1 1 1', чтобы обычный черный консоли не пропадал как прозрачный. Для работы фичи необходим плагин ConEmu. Проверялось на черном фоне «User screen»; `[ ]` Always show menu bar; `[x]` Show key bar. При скрытии панели первая ячейка консоли в первой строке не скрывается, чтобы панели можно было легко вернуть мышкой.


## Build 100208 ##
#### Bugfixes ####
  * [Issue 176](https://code.google.com/p/conemu-maximus5/issues/detail?id=176): files dragged from archives has zero size.
#### Changes ####
  * В файле настроек xml можно использовать тип long (например `<value name="ToolbarAddSpace" type="long" data="24"/>`).
  * При полностью скрытом заголовке окно можно таскать за свободное место бара.
  * В интерфейс вынесены настройки TrueColorer, ExtendFonts, !bgImageColors.
  * Добавлен Drop из Windows Recycle Bin.


## Build 100207 ##
#### Bugfixes ####
  * В некоторых случаях ресайз панелей конфликтовал с плагином PanelTabs.
  * [Issue 174](https://code.google.com/p/conemu-maximus5/issues/detail?id=174): ConEmu.Maximus5.100206a Баг с отображением панели.
#### Changes ####
  * Некоторый перетрях страницы настроек.
  * Проверка значения "LoadConIme" выполняется только в Vista. В Windows7 проверка больше не производится.
  * Теперь при нажатии 'Save settings' в диалоге настроек сохраняются все настройки, а не только те, которые можно менять через интерфейс. Сделано для возможности полного переноса настроек в XML из реестра.
  * В 'Features' добавлен флажок 'Resize panels by mouse'. 3-е состояние можно использовать когда на Ctrl+Left/Right/Up/Down висят макросы. В этом случае изменение размеров происходит после отпускания кнопки мышки.
  * [Issue 164](https://code.google.com/p/conemu-maximus5/issues/detail?id=164): добавлен флажок 'Hide caption always'. Таскать окошко можно за верхнюю грань рамки.


## Build 100206 ##
#### Bugfixes ####
  * Не запускалось: conemu /cmd "C:\Program Files\FAR\far.exe".
  * [Issue 172](https://code.google.com/p/conemu-maximus5/issues/detail?id=172): ConEmu10020304: проблема с правым кликом на PanelTabs.
#### Changes ####
  * Теперь мышкой можно менять высоту и ширину панелей. При наведении курсора на левый край правой панели или низ правой/левой панели - курсор меняется на сплиттер. Размер панелей меняется **банальной отсылкой в консоль Ctrl+Left/Right/Up/Down. Если вы их перемакросили - cрабатывать будут ваши макросы**, а не смена размера. DblClick - выравнивает панели. Отключить можно настройкой "DragPanel".
  * Флажок 'Force monospace' заменен на 3-е состояние флажка 'Monospace'.


## Build 100204 ##
#### Bugfixes ####
  * Не работал Restart консоли.
  * Что-то не то было с активацией плагина в FAR 1.7x. Вроде починил...


## Build 100203 ##
#### Bugfixes ####
  * [Issue 167](https://code.google.com/p/conemu-maximus5/issues/detail?id=167): ConEmu100129: конфликт маленького BackGround Image и colorer.
  * [Issue 168](https://code.google.com/p/conemu-maximus5/issues/detail?id=168): null pointer exception when there is no taskbar
  * [Issue 170](https://code.google.com/p/conemu-maximus5/issues/detail?id=170): в некоторых случаях не определялся момент завершения операции.
  * Стили шрифта (ExtendFonts & FarColorer truemod) работали только при включенном 'Fix Far Borders'.
#### Changes ####
  * ConEmuC. Если выполняется 'пустая команда' (ассоциация '@' в фар, и т.п.), то cmd.exe не запускается, высота буфера консоли не меняется.
  * Minor changes in console resize.
  * Minor changes in %ComSpec% /c.


## Build 100201 ##
#### Changes ####
  * Поддержка стиля 'Underline' в FarColorer truemod.


## Build 100129 ##
#### Bugfixes ####
  * Поломалась смена шрифта 'на лету'.
  * Плагин принудительно отображал скрытое окно FAR при запуске в Console2.
  * Ошибочно включался Drag их некоторых меню (плагинов).
#### Changes ####
  * Улучшения при детаче: создание нового буфера True-Colorer, по завершении выполнения команды в отключенной консоли отображается 'Press Enter to close console', восстанавливаются табы.
  * Internal. Некоторые изменения в механизме перехвата функций.
  * FarColorer truemod. Поддержка новой версии и стилей шрифта (bold/italic/bold&italic).


## Build 100125 ##
#### Changes ####
  * Улучшена отрисовка курсивом.
  * FarColorer truemod ready.
  * После детача FAR (CtrlAltTab) плагин автоматически цепляет созданное окно в новую вкладку ConEmu.


## Build 100116 ##
#### Bugfixes ####
  * Зависание при переключении в консоль, в которой висит 'Press Enter to close console...'.
  * [Issue 161](https://code.google.com/p/conemu-maximus5/issues/detail?id=161): Тормоза при AutoTabs.
  * [Issue 159](https://code.google.com/p/conemu-maximus5/issues/detail?id=159): Can't determine a value of 'LoadConIme'...
#### Changes ####
  * Во время запуска ConEmu или создания консоли в первой строке консоли отображается текущий статус.
  * [Issue 73](https://code.google.com/p/conemu-maximus5/issues/detail?id=73): Добавлен флажок 'Hide caption when maximized'. При его включении, максимизированный режим (AltF9) отличается от FullScreen только видимостью панели задач.
  * [Issue 142](https://code.google.com/p/conemu-maximus5/issues/detail?id=142): В настройку шрифта добавлен флажок 'Auto'. При включении и ресайзе окна - меняется размер шрифта, а размер консоли в символах остается неизменным.
  * Скорректирован список размеров растровых шрифтов (теперь отображаются все установленные, с высотой не менее 8).

## Build 100116 ##
  * ConEmu теперь может работать в 'портабельном' режиме. Для включения 'портабельности' переименуйте файл 'ConEmu\_Sample.xml' в 'ConEmu.xml'. Переименовывать можно после загрузки ConEmu, в этом случае в XML можно сохранить настройки, загруженные из реестра.
#### Bugfixes ####
  * Правки инициализации на 'чистых' настройках.
  * Поправлена GCC сборка (xml в GCC не поддерживается).
  * При невозможности создать требуемый шрифт (заданный в настройке шрифт отсутствует в системе) отображается сообщение об ошибке, а для шрифта рамок используется 'Lucida Console'.
  * [Issue 90](https://code.google.com/p/conemu-maximus5/issues/detail?id=90): В некоторых случаях терялось положение левого верхнего угла окна.
  * Немного поправлена прокрутка ползунком.
  * В некоторых случаях в FullScreen не скрывалась панель задач.


## Build 100113 ##
#### Bugfixes ####
  * [Issue 156](https://code.google.com/p/conemu-maximus5/issues/detail?id=156): Поломалось сохранение строковых параметров.


## Build 100112 ##
#### Bugfixes ####
  * [Issue 154](https://code.google.com/p/conemu-maximus5/issues/detail?id=154): Моргание заголовка окна.
  * [Issue 155](https://code.google.com/p/conemu-maximus5/issues/detail?id=155): Не отключалась опция "Enhance progressbars and scrollbars".
  * Утечки при D&D.
#### Changes ####
  * Одновременное использование шрифта Normal/Bold/Italic. Жирность/курсивность управляется цветом фона. Настраивается через реестр ("ExtendFonts", "ExtendFontBoldIdx", "ExtendFontItalicIdx", "ExtendFontNormalIdx").


## Build 100110 ##
#### Bugfixes ####
  * [Issue 151](https://code.google.com/p/conemu-maximus5/issues/detail?id=151): Моргание иконки "Администратора" в закладке.
  * Some fixes in ConEmuC.


## Build 100109 ##
#### Bugfixes ####
  * Исправлена отрисовка нац.символов OEM-ными шрифтами.
  * [Issue 149](https://code.google.com/p/conemu-maximus5/issues/detail?id=149): 091229.x64 - ConEmuC висит 10 секунд по Ctrl+Enter.
#### Changes ####
  * Улучшена работа с пропорциональными шрифтами. Ширина, указываемая у флажка 'Force Monospace' теперь учитывается даже при его отключении.
  * Оптимизация отрисовки.
  * Для любителей консольных растровых шрифтов в списке основных шрифтов дополнительно отображаются предопределенные "`[Raster Fonts 8x9]`", и т.п. **Warning!** При их выборе можно забыть о юникодных символах.

## Build 091229 ##
#### Changes ####
  * Опция 'Extend visualizer' погибла.
  * Главное окно ConEmu можно делать прозрачным (настройка на вкладке 'Colors'). При включении прозрачности отрисовка несколько замедляется.
  * Экран "замерзал", если программа (telnet, VBinDiff, "clr:") создавала свой консольный буфер. В Windows7 пока не работает.


## Build 091226 ##
  * Переделан способ передачи содержимого консоли в GUI.
#### Bugfixes ####
  * В некоторых случаях цветовые атрибуты отображались с задержкой.
  * Исправлены некоторые проблемы запускатора (COMSPEC).
  * Флажок 'Monospace' (не путать с 'Force monospace') работал неправильно.
#### Changes ####
  * Прозрачная поддержка консольных алиасов для cmd.exe/conemuc.exe.
  * Если FAR чем-то сильно занят (не опрашивается клавиатура) включается курсор с часиками (если курсор находится в рабочей области окна). Опцию можно отключить (флажок "Hourglass") или настроить время задержки (реестр, "FarHourglassDelay").
  * Debug: для запуска консольных команд в обход ComSpec можно использовать префикс: "conemu:run:<xxx yyy ...>"
  * "…" в заголовке таба строго в конце.
  * Поддержка панелей различной высоты.
  * Настройка ("bgImageColors") для указания цветов фона, замещаемых на "Background image".
  * Все PE-файлы теперь подписаны, сертификат можно загрузить здесь: http://conemu-maximus5.googlecode.com/files/ConEmu.cer.7z


## Build 091206 ##
#### Bugfixes ####
  * Ctrl+Wheel - листание страницами в BufferHeight.
  * Не работало колесо в некоторых моделях мышек (короткие скроллы и прочее).
  * Признак администратора не отображался, если сам conemu.exe был запущен "От имени...".
#### Changes ####
  * Вместо суффикса " (Admin)" в табе отображается значок "Щит". Суффикс и значок можно настроить через реестр.
  * Уменьшена задержка перед началом D&D.
  * Поддержка изменений заголовка FAR2 (не работали RClic, drag, ...).
  * Устранена некоторая задержка при выходе из FAR через F10.

## Build 091201 ##
  * Не учитывался макрос, задаваемый в "RightClickMacro2".


## Build 091128 ##
#### Bugfixes ####
  * Лечим разнообразные наведенные глюки версии 07.
  * В некоторых случаях при закрытии ошибочно отображалось предупреждение о наличии несохраненных редакторов.
  * В некоторых случаях при запуске cmd в новой консоли не включался режим прокрутки.
#### Changes ####
  * В папке 'Plugins\ConEmu' теперь выкладываются обе версии плагина (x64 и x86) для упрощения установки и использования.
  * Ускорена работа "RClick4ContextMenu", снято требование к кнопке Apps или настройке макроса.
> > Возможность настройки макроса осталась, но за ненадобностью переименована в "RightClickMacro2".
> > (Для информации) Макрос по умолчанию теперь такой:
> > $If (CmdLine.Empty) %Flg\_Cmd=1; %CmdCurPos=CmdLine.ItemCount-CmdLine.CurPos+1; %CmdVal=CmdLine.Value; Esc $Else %Flg\_Cmd=0; $End $Text "rclk\_gui:" Enter $If (%Flg\_Cmd==1) $Text %CmdVal %Flg\_Cmd=0; %Num=%CmdCurPos; $While (%Num!=0) %Num=%Num-1; !CtrlS $End $End
  * Добавлена возможность одновременного запуска нескольких консолей.
> > Пример: conemu.exe /cmd @startfile.txt
> > В файле каждая строка соответсвует запускаемой команде.
> > Допустимо указание в строке параметра /BufferHeight.
> > Если в начале строки стоит ">" - эта консоль будет активной после завершения запуска.
> > Если в начале строки стоит "`*`" - эта консоль будет запущена под администратором.
> > Пример файла startfile.txt (лидирующие пробелы и табуляции игнорируются):
```
>E:\Source\FARUnicode\trunk\unicode_far\Debug.32.vc\far.exe
*/!BufferHeight 400 cmd
/!BufferHeight 1000 powershell
```
  * FAR2 only. Добавлена экспериментальная фича - ASCII сортировка в FAR (в основном для файлов). Вроде работает, но используйте на свой страх и риск ;) Смысл в том, что при сравнении символов ASCII \x01..\x7F используется простое сравнение (регистрозависимо или независимо).
  * Добавлен флажок "Fix Alt on AltTab". По умолчанию - отключен.
  * В FAR2 больше не используется hotkey плагина. Собственно в FAR2 и макросы F14/CtrlF14/AltF14/ShiftF14 больше не нужны.
  * В макросе ShiftEnter учитывается диалог быстрого поиска в панелях.
  * Смена способа отрисовки окна (дочернее окно в котором ранее шла отрисовка теперь невидимое). Больше не должно быть задержек при появлении/пропадании табов.
  * Теперь можно включать и выключать режим "BufferHeight" на лету. Для смены нажмите кнопку режима на toolbar'е. Отображается подтверждение.
  * Опция 'GUI progress bars' погибла, вместо нее добавлена 'Enhance progressbars and scrollbars' (ранее соответствующая 3-му состоянию флажка 'Fix FAR borders').


## Build 091107 ##
#### Bugfixes ####
  * В некоторых случаях после завершения консольных команд не восстанавливалась высота консоли (оставалась прокрутка).
  * В некоторых случаях графическое EMenu могло всплывать под ConEmu.
#### Changes ####
  * Параметр "FarSyncSize" в настройках.
  * При правом щелчке по табу не относящемуся к FAR (cmd, powershell, и т.п.) отображается диалог с кнопками Restart/Terminate/Cancel.
  * Добавлено распознавание прогресса (процентов выполнения) в текущей строке текста консоли (chkdsk, wget, и т.п.)
  * Обновлено распознавание прогресса в заголовке для плагина 7-zip alt.


## Build 091031 ##
#### Bugfixes ####
  * Не работал параметр запуска '/BufferHeight'.
  * Лечим ресайз для режима с прокруткой.
  * В некоторых случаях при листании шрифтов в настройках сбивался Charset.
  * Некорректное создание OEM шрифтов.
  * Правки ресайза при автотабах.
#### Changes ####
  * На вкладке настроек поле 'Charset' динамически обновляется при смене шрифта.
  * Добавлен 'ответ' в FAQ на счет растровых шрифтов консоли.


## Build 0910xx ##
#### Bugfixes ####
  * При "AltTab AltTab" в консоль проваливался 'чистый' Tab. Теперь в консоль, при активном FAR, посылается AltTab во избежания срабатывания макросов, повешенных на Alt (RAlt).
  * Задержка 10 секунд при запуске 16битных приложений, сбивался размер окна.
  * Не работал символ "c" в запущеном cmd.exe


## Build 091001 ##
  * Ошибочно включался Drag, если выделенных элементов на панели не было и текущий элемент - '..'.
  * При выполнении консольных команд дублировался Ctrl-C (<sup>C</sup>C).
  * Некорректное отображение символов с кодами ASCII < 32.
  * Не обновлялась подсказка у иконки в трее.
  * Утечка хэндлов во второй копии ConEmu.
  * Падение при перетаскивании мышки в заголовок окна.
#### Changes ####
  * По умолчанию Affinity не меняется.
  * В FAR2 размер консоли изменяется через Synchro. Соответственно несколько меняется поведение при автотабах.


## Build 090923 ##
#### Changes ####
  * Поддержка изменений Multimedia Viewer.

## Build 090922 ##
#### Bugfixes ####
  * Исправлена ошибка переключения раскладки (зеленый индикатор RusLat).
#### Changes ####
  * В заголовке консоли/табе/индикаторе отображаются проценты выполнения некоторых консольных программ (nerocmd).


## Build 090919 ##
#### Bugfixes ####
  * В заголовке окна, при отключенных табах, не обновлялось количество консолей "`[n/n]`".
  * При 'закрытии' таба правым щелчком мышки FAR2 не отрисовывал диалоги подтверждения пока не дернешь мышкой.
#### Changes ####
  * При отключенных табах 'Recent mode' игнорируется. Окна переключаются последовательно.
  * Теперь можно настроить шрифт табов (пример в Settings-ConEmu.reg). Например, для корректного отображения иероглифов нужно выбрать шрифт типа 'Arial Unicode MS'.


## Build 090915b ##
#### Bugfixes ####
  * Могли злобно виснуть (или падать?) при попытке ресайза.
  * В FAR2 активация плагина переведена на ACTL\_SYNCHRO. Теперь в макросе для EMenu можно использовать перфикс '@'.
> > Текстовое EMenu теперь всегда активируется по центру экрана
> > (Side effect - при нажатии клавиатуры скрытая реальная консоль сдвигается в сторону от мышки).
> > В текущей версии рекомендуются такие настройки EMenu:
> > Apps - показывает текстовое меню (автоматически отображается по центру окна).
> > Для мыши - макрос вида "RightClickMacro"="@F11 e Enter Enter" в настройках ConEmu.


## Build 090913 ##
#### Bugfixes ####
  * При выполнении консольных команд в закладке отображался заголовок панели вместо выполняемой команды (при наличии редактора/вьювера).
  * В FAR2 не работал макрос !CtrlShiftT.
  * Некорректная обработка слишком быстрых изменений. В частности, прокрутка вьювера левой кнопкой мышки приводила к безостановочному скроллингу.
  * Правки некорректной высоты буфера консоли при старте (пока только FAR).
#### Changes ####
  * На время выполнения консольных команд прячутся все закладки редакторов/вьюверов текущей консоли.
  * ConEmuC в режиме подмены %ComSpec% теперь передает в FAR код возврата запущенной программы.
  * Информация о реальных размерах созданных шрифтов на вкладке 'Info' настроек.
  * Если в макросе (пока это только RightClickMacro и TabCloseMacro) первый символ '@' и после него не пробел - макрос выполняется с флагом KSFLAGS\_DISABLEOUTPUT. В FAR2 эффекта пока не имеет.
  * На вкладке 'Features' диалога 'Settings' добавлен выпадающий список высот для 16-битных приложений.
  * На вкладке 'Features' диалога 'Settings' добавлена возможность принудительного (или автоматического) указания режима вывода для cmd.exe (ключи /U и /A командного процессора). При выборе Automatic в FAR2 активируется Unicode вывод, а в FAR1.7x - OEM.
  * Улучшена работа ключа /single. В FAQ добавлен вопрос про одновременный запуск двух вкладок.
  * Для удобства в файлы !CtrlO.reg и !CtrlO\_View.reg добавлен макрос !CtrlShiftO гасящий/показывающий панели (стандартное поведение FAR по !CtrlO).
  * В диалог создания новой консоли добавлена история команд.


## Build 090909 ##
#### Bugfixes ####
  * В меню плагина под Far 1.7x отображался мусор.
  * Не работали дополнительные языковые раскладки (например US-Dvorak).
  * Вроде починился баг с открытием новой консоли с некорректным размером.
  * Исправлена отрисовка рамок в случае когда шрифт рамок был шире основного.
#### Changes ####
  * Упрощен выбор цветовых схем (вкладка 'Colors' в диалоге настроек). Если есть чем поделиться с общественностью - присылайте настройки :)
  * При завершении консоли активной становится предыдущая активная.
  * Перед закрытием ConEmu (щелчком по крестику в заголовке окна) отображается предупреждение при наличии несохраненных редакторов или незавершенных процессов (копирования и пр.).


## Build 090906 ##
#### Bugfixes ####
  * Окно не разворачивалось, если было свернуто из полноэкранного режима.
  * В FAR2 отвалились макрос !CtrlO (нужно обновить reg-файл из поставки).
  * В FAR 1.7x при закрытии последнего редактора/вьювера в табах вместо панели отображался уже закрытый редактор/вьювер (появилось в 2009.08.18).
  * При запуске 'Run as administrator' в Win7 реальная консоль оставалась видимой, а CtrlWinAltSpace для нее не работал.
  * При закрытии неактивной консоли не обновлялся список табов.
  * В Win7 не работало закрытие консолей по крестику, если консоль была запущена 'Run as administrator'.
#### Changes ####
  * В процессе FAR'а запрещено менять переменную ComSpec (во избежание, плагин FarCall ее уже нарушал).
  * Для консолей запущенных 'Run as administrator' в заголовке (ConEmu) и табах отображется суффикс. Можно настроить через реестр ("AdminTitleSuffix"). По умолчанию - " (Admin)".
  * Версия для 64-битных ОС. Если в 64-битной ОС используется 32-битный фар - плагин для него нужно брать из 32-битной сборки ConEmu.
  * На странице 'Colors' диалога настроек добавлена возможность выбрать одну из 'умолчательных' цветовых схем.


## Build 090829 ##
#### Bugfixes ####
  * В некоторых случаях (ShiftEnter на far.exe) не отключалось требование 'Press Enter to close console...' при быстром выходе из FAR.
  * Некоторые изменение в общении с плагином/сервером.


## Build 090818 ##
#### Bugfixes ####
  * Не работало автоскрытие табов при открытии модального редактора в FAR 1.7x.
  * В некоторых случаях оставались 'левые' табы (часто проявлялось при открытии модальных редакторов).


## Build 090817 ##
#### Bugfixes ####
  * В некоторых случаях признак модификации редактора (`*`) в табе изменялся не сразу.
#### Changes ####
  * Отложенное обновление табов выполняется только в третьем состоянии флажка 'Enable Tabs'.


## Build 090814 ##
#### Bugfixes ####
  * В некоторых случаях, после завершения 16битной программы сбивался размер консоли.
  * В некоторых случаях после нормального запуска FAR (например, через ShellExecute) не отключалось требование 'Press Enter to close console...' при быстром выходе из FAR.
  * D&D. Иконки и имена перетаскиваемых файлов не отображались при быстром вытаскивании наружу.
  * D&D. Положение курсора над оверлеем при длинных именах файла.
#### New ####
  * В диалоге (пере)запуска консоли (Win-W или Win-~) добавлена возможность указать текущую директорию для нового процесса.
  * При выполнении макросов табы в ConEmu обновляются отложенно, по завершению макроса.


## Build 090810 ##
#### Bugfixes ####
  * Утечка ресурсов (Pipes\ConEmuGuiXXX) в ConEmuC.
  * При запуске 'telnet.exe' GUI 'замерзал' (телнет можно было видеть только в [RealConsole](ConEmuTerms#RealConsole.md)).


## Build 090725 ##
#### Bugfixes ####
  * AltTab fixed. В консоль проваливалось нажатие Alt, что могло приводить к выполнению макроса.
  * Исправлены ошибки аттача ФАРА (запущенного не из ConEmu) к GUI. ConEmuC падал. Включался BufferHeight.
#### New ####
  * Если при дропе в ком.строку удерживать модификатор (shift, alt или ctrl) в начало будет автоматически добавлен 'goto:'. Пуста ли ком.строка до выполнения дропа - не проверяется.
  * При драге в фаре отображаются иконки и имена выделенных папок/файлов.


## Build 090723 ##
  * При работе в редакторе моргал заколовок окна.
  * Немного причесан диалог настроек.


## Build 090722 ##
#### New ####
  * При драге ИЗ проводника отображаются иконки перетаскиваемых файлов (checked in XP).

## Build 090721a ##
#### Bugfixes ####
  * В версии 2009.07.19 отвалилась передача !CtrlC в FAR.
  * При 'Force monospace' не работало 3-d state 'Fix Far borders'.
  * Учтена баго-фича фара (например, при попытке выполнить макрос во время копирования экран 'замерзает'). Макросы, вызывающиеся из ConEmu, теперь вызываются без флага KSFLAGS\_DISABLEOUTPUT.
#### Changes ####
  * Уточнения в сообщении при выходе из ConEmuC.
  * Если путь к ConEmuC.exe содержит пробелы (Program Files, etc.) то при установке переменной окружения ComSpec путь конвертируется в короткие имена.
  * Макросы для FAR2 переведены на callplugin.
  * Если при создании новой консоли удерживать Shift - появится диалог подтверждения (с флажком 'Run as administrator').
  * Заработал флажок 'Run as administrator'. Теперь в Vista и выше  можно быстро (пере)запустить консоль с максимальными правами (UAC), а в !WinXP - под другим пользователем.


## Build 090719 ##
### Warning ###
  * Изменен способ передачи/обработки нажатий клавиш. Что-то могло поломаться.
#### Changes ####
  * Табы. Если при переключении (CtrlTab) удерживать Alt - режим переключения (Recent mode) инвертируется.


## Build 090717 ##
#### Bugfixes ####
  * Исправлено колесо мышки.
  * Не работали табы и драгдроп при запуске: ConEmu.exe /cmd cmd.exe /K far.bat
  * Исправлена отрисовка фоновой картинки.
#### New ####
  * Все найденные в папке с ConEmu .ttf файлы шрифтов временно регистрируются для использования в системе. Если в реестре настройка шрифтов остутствует - программа выбирает один из зарегистрированных шрифтов вместо стандартного 'Lucida Console'.


## Build 090716 ##
#### Bugfixes ####
  * Лечим артефакты отрисовки (ClearType).
  * Force monospace. Ширина шрифта не передергивалась при изменении флажка 'Force monospace'.
  * Немного поправлен алгоритм определения 'темности' фона при отрисовке курсора.
  * Событие MouseButtonUp проходило в консоль только если курсор был в клиентской области ConEmu.
#### New ####
  * DragDrop. Если папка рабочего стола отсутсвует (!WinPE?) выдается осмысленное сообщение. В этом случае DragDrop должен работать, но без возможности создания ярлыков удерживанием кнопки Alt.
  * DragDrop. Разрешен Drop в область командной строки.
  * Пытаемся добавить флажок 'Run as administrator' в диалог подтверждения создания (пересоздания) консоли. Пока работает нестабильно - при его включении ConEmu может валиться.


## Build 090711a ##
#### Bugfixes ####
  * Падение ConEmu при выполнении длинных команд, содержащих амперсанды (&).
  * Исправлена отрисовка курсора на прокрутке и прогрессе в третьем состоянии флажка 'Fix far borders'.
  * При возникновении ошибки 'The requested operation requires elevation' программа запускается немодально в новой консоли, не подключенной к ConEmu.
  * Правки в Manifest.
#### Changes ####
  * По окончанию загрузки в консоли FAR (точнее, при загрузке плагина ConEmu) отключается требование 'Press Enter to close console...'.
  * Изменен алгоритм цвета курсора. При отключенном флажке 'Color' курсор белый (точнее color#15) на темном фоне и черный (color#0) на светлом фоне. При включенном флажке 'Color' курсор отрисовывается как в стандартной консоли.

## Build 090709a ##
#### Bugfixes ####
  * Проблемы при закрытии консолей, после открытия максимального их количества (12).
  * Поправлена активация с клавиатуры Win-F11/F12.
  * При закрытии/пересоздании консоли пропадал таб этой консоли.
  * В некоторых случаях не работало ограничение FPS.
  * Небольшие правки ресайза (Tabs, Alt-F9).
#### Changes ####
  * При невозможности создать новую консоль (их уже максимальное количество) отображается предупреждение.
  * Переменные окружения. '!ConEmuHWND' удаляется (плагином) при детаче консоли или аварийном завершении графической части. Добавлена переменная 'ConEmuDir'.
  * Возвращен 'cmd\_autorun.cmd'. 'Прицепить' к GUI любую консоль можно выполнив в ней команду 'conemuc /ATTACH /NOCMD'.
  * В меню плагина добавлен пункт 'Attach to ConEmu'. Для тех случаев, когда был детач (CtrlAltTab) или FAR был запущен отдельно.

## Build 090707 ##
#### Bugfixes ####
  * После запуска и закрытия 16битного приложения замедлялась реакция на изменения в консоли.
  * Неправильно обрабатывалась высота консоли в 16битном режиме. Сейчас поддерживаются все стандартные высоты (25,28,43,50). Перед запуском 16битного приложения может быть принудительно установлена высота, задаваемая в настройке ("16bit Height").

#### Changes ####
  * Правки в GCC мейке.
  * Разрешена смена размера консоли самим консольным приложением (например, командой 'MODE CON: COLS=80 LINES=25').

## Более старые записи ##
  * [Полный лог изменений на SVN](https://conemu-maximus5.googlecode.com/svn/trunk/ConEmu-alpha/Release/ConEmu/WhatsNew-ConEmu.txt)