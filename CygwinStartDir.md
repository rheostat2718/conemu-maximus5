﻿#summary cygwin, mingw, ConEmu and start up directory

# ConEmu has moved to conemu.github.io #
New versions of this file are available on [conemu.github.io](http://conemu.github.io/en/CygwinStartDir.html).

# Cygwin startup directory #
One of the most asked questions about cygwin using - how to run its shell in the specified working directory.

Finally, thanks to csbdess, [Issue1552](https://code.google.com/p/conemu-maximus5/issues/detail?id=1552)#15, the problem was identified.

Cygwin always do ‘`cd "${HOME}"`’ from `/etc/profile`.
```
# Make sure we start in home unless invoked by CHERE
if [ ! -z "${CHERE_INVOKING}" ]; then
  unset CHERE_INVOKING
else
  cd "${HOME}" || echo "WARNING: Failed attempt to cd into ${HOME}!"
fi
```
So, if you do not like that behaviour, read below for solutions.

**Note**. MinGW (at least version installed on my PC) does not have checking for `CHERE_INVOKING` environment variable.
There is just `cd "$HOME"` at the end of `/MinGW/msys/1.0/etc/profile`.
You may change that file yourself to get the same checks as cygwin.

# CHERE #
Cygwin will not do ‘`cd "${HOME}"`’ if environment variable `CHERE_INVOKING` is defined.
So, if you run your shell (bash) as following, shell will be started in proper working directory.

```
set CHERE_INVOKING=1 & c:\cygwin\bin\sh.exe --login -i
```

Thought, you can, but must not change the contents of your `/etc/profile` commenting ‘`cd "${HOME}"`’...
Because you can change cygwin behaviour within ConEmu tasks or command line.

# ConEmuWorkDir #

Another workaround - do `cd` to the working folder in your shell profile (`.bashrc`, `.zshrc`, etc.)

```
if [ -n "${ConEmuWorkDir}" ]; then
  cd "$ConEmuWorkDir"
fi
```

ConEmu set `ConEmuWorkDir` to the console startup directory, so you may use it in your scripts.
Read more about ConEmuEnvironment in wiki.

<a href='Hidden comment: 
Portable to all POSIX shells:

if [ -n "${foobar+1}" ]; then
echo "foobar is defined"
else
echo "foobar is not defined"
fi
Make that ${foobar:+1} if you want to treat foobar the same way whether it is empty or not defined. You can also use ${foobar-} to get an empty string when foobar is undefined and the value of foobar otherwise (or put any other default value after the -).

In ksh, if foobar is declared but not defined, as in typeset -a foobar, then ${foobar+1} expands to the empty string.

Zsh doesn"t have variables that are declared but not set: typeset -a foobar creates an empty array.

In bash, arrays behave in a different and surprising way. ${a+1} only expands to 1 if a is a non-empty array, e.g.

typeset -a a; echo ${a+1}    # prints nothing
e=(); echo ${e+1}            # prints nothing!
f=(""); echo ${f+1}          # prints 1
The same principle applies to associative arrays: array variables are treated as defined if they have a non-empty set of indices.

A different, bash-specific way of testing whether a variable of any type has been declared is to check whether it"s listed in ${!PREFIX*}:

case " ${!foobar*} " in
*" foobar "*) echo "foobar is declared";;
*) echo "foobar is not declared";;
esac
This is equivalent to testing the return value of typeset -p foobar or declare -p foobar.

In bash, like in ksh, set -o nounset; typeset -a foobar; echo $foobar triggers an error in the attempt to expand the undefined variable foobar. Unlike in ksh, set -o nounset; foobar=(); echo $foobar (or echo "${foobar[@]}") also triggers an error.

Note that in all situations described here, ${foobar+1} expands to 1 if and only if $foobar would cause an error under set -o nounset.
'></a>


# cygdrive #
Not related to working directory and ConEmu, actually, but let it be here.

If you want to remove annoying `/cygdrive`, just replace one line in your `/etc/fstab`.

```
#none /cygdrive cygdrive binary,posix=0,user 0 0
none / cygdrive binary 0 0
```