@echo off
xgettext --from-code=utf-8 -C -n -k_ -o Data\Locales\appetizer.pot *.cpp
xgettext --from-code=utf-8 -C -n -k_ -j -o Data\Locales\appetizer.pot utilities\*.cpp
xgettext --from-code=utf-8 -C -n -k_ -j -o Data\Locales\appetizer.pot gui\*.cpp
msgmerge -U Data\Locales\fr\appetizer.po Data\Locales\appetizer.pot
msgmerge -U Data\Locales\en\appetizer.po Data\Locales\appetizer.pot
msgmerge -U Data\Locales\de\appetizer.po Data\Locales\appetizer.pot
msgmerge -U Data\Locales\nl\appetizer.po Data\Locales\appetizer.pot
msgmerge -U Data\Locales\pl\appetizer.po Data\Locales\appetizer.pot
