         K   J        ��������SӘ�\�u��oC��ȋ6            u#!/bin/sh

dir=`dirname $0`

$dir/filter_stderr | sed "s/ line [0-9]*://"
