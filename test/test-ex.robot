*** Settings ***
Documentation	Testcases for syncped ex
Test Setup	Test Setup
Suite Setup	Suite Setup
Suite Teardown	Suite Teardown
Resource	keywords.resource


*** Test Cases ***
edit
	Input	:e other.txt
	...	:f
	Syncped
	# the :e command is handled by event, so other.txt not yet active
	Output Contains	${file-startup}

empty
	Input	:1000
	...	:.=
	Syncped
	Output Contains	1

info
	Input	:a|line has text
	...	:f
	Syncped
	Output Contains	${file-startup}
	Output Contains	1
	Output Contains	%
	Output Contains	level

mdi
	Input	:a|line has text
	...	:e other.txt
	...	:e more.txt
	# We cannot test mdi, because of events.
	Syncped

process
	Input	:!pwd
	Syncped
	Output Contains	syncped

saveas
	${size}=	Get File Size	test-ex.robot
	Input Many	:e test-ex.robot	1
	Input Many	:w copy.txt	1
	Syncped	5
	File Should Exist	copy.txt
	${size-copy}=	Get File Size	copy.txt
	Should Be Equal	${size}	${size-copy}
	Remove File	copy.txt

set
	Input	:set all *
	Syncped
	Output Contains	ts=

set-bool
	Input	:set nosws *
	...	:set sws ? *
	Syncped
	Output Contains	nosws

set-info
	Input	:set ts ? *
	Syncped
	Output Contains	ts=

set-verbosity
	Input	:set ve?
	Syncped
	Output Contains	ve=${level}

substitute
	Input	:a|line has text
	...	:a|line has a tiger
	...	:a|line has simon and simon and garfunkel
	...	:%s/simon/nick/
	Syncped
	Output Contains	1
	Output Contains	simon

substitute-global
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
