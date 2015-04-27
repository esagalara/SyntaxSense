# SyntaxSense

A Notepad++ plugin for auto-detecting content language, for syntax highligting junkies.
This is usesful when working with files that contain XML or HTML but have different endings, such as .config and .aspx.

## Build
Use Visual Studio with C++ to build solution. I used VS 2013.

## Deploy
Copy SyntaxSense.dll to Programs\Notepad\plugins

Copy SyntaxSense.ini to Users\ ...\AppData\Roaming\Notepad++\plugins\config

## Configure
SyntaxSense uses *regular expressions* for matching the editor content to a known language.

Example:
```
[xml]
LangCode = 9
RegEx = <\?\s*xml\s*version=\"\S+\".*?>
Flags = 0
```
LangCode is Notepad++'s internal language designator for XML.
Regex is the regular expression for matchin the start of an XMl file
Flags are for altertering how the regex matches, such as ignoring case.
