*** Settings ***
Documentation	Testcases for syncped ex
Test Setup	Test Setup
Suite Setup	Suite Setup
Suite Teardown	Suite Teardown
Resource	keywords.resource


*** Test Cases ***
EDIT
	Input	:e other.txt
	...	:f
	Syncped
	# the :e command is handled by event, so other.txt not yet active
	Output Contains	${file-startup}

EMPTY
	Input	:1000
	...	:.=
	Syncped
	Output Contains	1

INFO
	Input	:a|line has text
	...	:f
	Syncped
	Output Contains	${file-startup}
	Output Contains	1
	Output Contains	%
	Output Contains	level

MDI
	Input	:a|line has text
	...	:e other.txt
	...	:e more.txt
	# We cannot test mdi, because of events.
	Syncped

PROCESS
	Input	:!pwd
	Syncped
	Output Contains	syncped

SET
	Input	:set all *
	Syncped
	Output Contains	ts=

SET-BOOL
	Input	:set nosws *
	...	:set sws ? *
	Syncped
	Output Contains	nosws

SET-INFO
	Input	:set ts ? *
	Syncped
	Output Contains	ts=

SET-VERBOSITY
	Input	:set ve?
	Syncped
	Output Contains	ve=${level}

SUBSTITUTE
	Input	:a|line has text
	...	:a|line has a tiger
	...	:a|line has simon and simon and garfunkel
	...	:%s/simon/nick/
	Syncped
	Output Contains	1
	Output Contains	simon

SUBSTITUTE-GLOBAL
	Input	:a|line has text
	...	:a|line has a tiger
	...	:a|line has simon and simon and garfunkel
	...	:%s/simon/nick/g
	Syncped
	Output Contains	2
	Output Contains	simon
	Contents Does Not Contain	simon

*** Comments ***
Copyright: (c) 2020-2021 Anton van Wezenbeek
