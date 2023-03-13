#!/bin/bash

xgettext \
	--package-name=fcitx5-array \
	--c++ \
	--from-code=UTF-8 \
	-k_ \
	-kN_ \
	-o po/fcitx5-array.pot \
	src/engine.cpp src/engine.h src/arrayconfig.h

xgettext \
	--package-name=fcitx5-array \
	--language=Desktop \
	-k \
	--keyword=Name \
	--keyword=GenericName \
	--keyword=Comment \
	--keyword=Keywords \
	-j \
	-o po/fcitx5-array.pot \
	src/array.conf.in src/array-addon.conf.in.in

## To re-generate from scratch:
#msginit -l zh_TW.UTF-8 --no-translator -o po/zh_TW.po -i po/fcitx5-array.pot
#msginit -l zh_TW.UTF-8 --no-translator -o po/zh_CN.po -i po/fcitx5-array.pot

msgmerge --update po/zh_TW.po po/fcitx5-array.pot
msgmerge --update po/zh_CN.po po/fcitx5-array.pot
