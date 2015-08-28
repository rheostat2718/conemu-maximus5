=Types of High-DPI OS settings=
There are two ways how high-dpi support may work in Windows.

# Types of high-dpi awareness #

There are three style of DPI support in modern Windows versions.


## Non-dpi-aware ##
These old applications do not know anything about monitor dpi and how to deal with high resolution.

Windows tries to make them as accessible as possible so their windows are shown larger
than their ‘actual’ sizes so they fit your screen more properly.

On the other hand that is done by simple upscaling so their client contents as bitmap
which causes blurred image.

#### Very old build without dpi-awareness causes blurred text ####
<img src='http://conemu-maximus5.googlecode.com/svn/files/dpi-old-150.png' title='Very old ConEmu build without dpi-awareness'>

<h4>New dpi-aware build shows clean picture</h4>
<img src='http://conemu-maximus5.googlecode.com/svn/files/dpi-new-150.png' title='New dpi-aware ConEmu build'>