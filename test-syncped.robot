*** Settings ***
Documentation	Testcases for syncped
Test Setup	Test Setup
Suite Setup	Suite Setup
Suite Teardown	Suite Teardown
Library	OperatingSystem
Library	Process


*** Variables ***
# Normally syncped exits after each test, override this
# variable on the commandline using '-v quit:0' to remain active
${quit}	1

# Normally syncped runs in quiet mode, only errors are logged,
# override this variable using '-v level:0' to run in verbose mode
# (only shown in the log file).
${level}	4

${SYNCPED}

${file-config}	test.json
${file-contents}	contents.txt
${file-input}	input.txt
${file-output}	output.txt
${file-startup}	empty.txt
${file-stdout}	stdout.txt


*** Test Cases ***
CMDLINE-HELP
	[Documentation]	Check whether we can startup correctly
	${result}=	Run Process	${SYNCPED}	-h
	# required by OpenGroup
	Should Contain	${result.stdout}	-c
	Should Contain	${result.stdout}	-R
	Should Contain	${result.stdout}	-s
	Should Contain	${result.stdout}	-t
	# our own
	Should Contain	${result.stdout}	--ex
	Should Contain	${result.stdout}	-j
	Should Contain	${result.stdout}	-V
	Should Contain	${result.stdout}	-X
	Should Contain	${result.stdout}	version

CMDLINE-LEXERS
	[Documentation]	Check whether we have at least a rfw lexer
	${result}=	Run Process	${SYNCPED}	-L
	Should Contain	${result.stdout}	rfw

# ex tests

EX-EDIT
	Input	:e other.txt
	...	:f
	Syncped
	# the :e command is handled by event, so other.txt not yet active
	Output Contains	${file-startup}

EX-EMPTY
	Input	:1000
	...	:.=
	Syncped
	Output Contains	1

EX-INFO
	Input	:a|line has text
	...	:f
	Syncped
	Output Contains	${file-startup}
	Output Contains	1
	Output Contains	%
	Output Contains	level

EX-MDI
	Input	:a|line has text
	...	:e other.txt
	...	:e more.txt
	# We cannot test mdi, because of events.
	Syncped

EX-PROCESS
	Input	:!pwd
	Syncped
	Output Contains	build

EX-SET
	Input	:set all *
	Syncped
	Output Contains	ts=

EX-SET-BOOL
	Input	:set nosws *
	...	:set sws ? *
	Syncped
	Output Contains	nosws

EX-SET-INFO
	Input	:set ts ? *
	Syncped
	Output Contains	ts=

EX-SET-VERBOSITY
	Input	:set ve?
	Syncped
	Output Contains	ve=${level}

EX-SUBSTITUTE
	Input	:a|line has text
	...	:a|line has a tiger
	...	:a|line has simon and simon and garfunkel
	...	:%s/simon/nick/
	Syncped
	Output Contains	1
	Output Contains	simon

EX-SUBSTITUTE-GLOBAL
	Input	:a|line has text
	...	:a|line has a tiger
	...	:a|line has simon and simon and garfunkel
	...	:%s/simon/nick/g
	Syncped
	Output Contains	2
	Output Contains	simon
	Contents Does Not Contain	simon

# vi tests

VI-CALCULATE
	Input	=9+9+9+9-2+(3*3)
	...	=2<4
	...	=128/2
	...	=127&7
	Syncped
	Output Contains	43
	Output Contains	32
	Output Contains	64
	Output Contains	7

VI-DEBUG	[Documentation]	Set a breakpoint, and give time to process it
	Input Many	:23	1
	Input Many	:de b	1
	Input Many	:1	1000
	Input Many	:100	1000
	Input	:1
	Syncped Debug
	Output Contains	lldb
	Output Contains	Breakpoint

VI-DELETE
	Input Many	:a|line	100
	Input	:1
	...	59dd
	Syncped
	Output Contains	59
	Output Contains	fewer

VI-DELETE-D
	Input	:a|line has some text
	...	:1
	...	ww
	...	D
	Syncped
	Contents Does Not Contain	some text

VI-FIND-NOT
	Input	:a|x
	...	:a|y
	...	:a|z
	...	/zz
	...	:.=
	Syncped
	Output Contains	4

VI-FIND-OK
	Input	:a|x
	...	:a|y
	...	:a|z
	...	/z
	...	:.=
	Syncped
	Output Contains	3

VI-INFO
	Input	:a|line has text
	...	
	Syncped
	Output Contains	1
	Output Contains	%
	Output Contains	level

VI-MACRO
	Input	@Template-test@
	Syncped
	Contents Does Not Contain	@Created@
	Contents Does Not Contain	@Date@
	Contents Does Not Contain	'Date'
	Contents Does Not Contain	@Datetime@
	Contents Does Not Contain	@Process@
	Contents Does Not Contain	@Year@
	
VI-MACRO-RECORD
	Input	:a|10 1123
	...	:1
	...	qb
	...	w
	...	"x
	...	yw
	...	b
	...	cw
	...	=x+100
	...	
	...	0
	...	j
	...	q
	...	:1
	...	@b
	Syncped
	Contents Contains	1223

VI-MARKER
	Input Many	:a|line has text	50
	Input	:10
	...	mx
	...	:1
	...	'x
	...	:.=
	Syncped
	Output Contains	10

VI-MODE-BLOCK
	Input Many	:a|line has text	50
	Input	:1
	...	w
	...	K
	...	10j
	...	w
	...	d
	Syncped
	Output Does Not Contain	10

VI-MODE-EX
	Input No Write	:vi
	Syncped Ex Mode

VI-MODE-INSERT
	Input	:a|one line
	...	ijjjjjjj
	Syncped
	Contents Contains	jjjjjjj

VI-MODE-VISUAL
	Input Many	:a|line has text	50
	Input	:1
	...	v
	...	10j
	...	d
	...	:1
	...	v
	...	35j
	...	d
	Syncped
	Output Contains	10
	Output Contains	35
	Output Contains	fewer
	Output Does Not Contain	9

VI-NAVIGATE
	Input Many	:a|line has text	50
	Input	:1
	...	jjjjjjj
	...	:.=
	Syncped
	Output Contains	8

VI-YANK
	Input Many	:a|line	100
	Input	:1
	...	59yy
	Syncped
	Output Contains	59
	Output Contains	yanked

VI-YANK-REGISTER
	Input Many	:a|line	10
	Input	:1
	...	"x
	...	yw
	...	:%d
	...	i
	...	xx
	...	
	Syncped
	Contents Contains	lineline


*** Keywords ***
Suite Setup
	Find Syncped
	Variable Should Exist	${SYNCPED}

Suite Teardown
	Remove File	${file-contents}
	Remove File	${file-input}
	Remove File	${file-output}
	Run Keyword If	${level} == 4	Remove File	${file-stdout}

Test Setup
	Create File	${file-input}
	Create File	${file-output}

Contents Contains
	[Arguments]	${text}
	${result}=	Get File	${file-contents}
	Should Contain	${result}	${text}

Contents Does Not Contain
	[Arguments]	${text}
	${result}=	Get File	${file-contents}
	Should Not Contain	${result}	${text}

Find Syncped
	${result}=	Run Process
	...	find	./
	...	-name	syncped
	...	-type	f
	Set Suite Variable	${SYNCPED}	${result.stdout}

Input
	[Arguments]	@{text}
	FOR	${cmd}	IN	@{text}
		Append To File	${file-input}	${cmd}
		Append To File	${file-input}	\n
	END

	Append To File	${file-input}	:w ${file-contents}\n
	IF	${quit} == 1
		Append To File	${file-input}	:q!
	END

Input Many
	[Arguments]	${line}	${count}
	[Documentation]	As Input, but extra argument for repeat count,
	...	and does not end write output and with quit
	FOR	${index}	IN RANGE	${count}
		Append To File	${file-input}	${line}
		Append To File	${file-input}	\n
	END

Input No Write
	[Arguments]	@{text}
	FOR	${cmd}	IN	@{text}
		Append To File	${file-input}	${cmd}
		Append To File	${file-input}	\n
	END

	IF	${quit} == 1
		Append To File	${file-input}	:q!
	END

Output Contains
	[Arguments]	${text}
	${result}=	Get File	${file-output}
	Should Contain	${result}	${text}

Output Does Not Contain
	[Arguments]	${text}
	${result}=	Get File	${file-output}
	Should Not Contain	${result}	${text}

Syncped
	[Documentation]	Runs syncped with suitable arguments
	Run Process
	...	${SYNCPED}
	...	-j	${file-config}
	...	-s	${file-input}
	...	-X	${file-output}
	...	-V	${level}
	...	${file-startup}
	...	stdout=${file-stdout}

Syncped Debug
	[Documentation]	Runs syncped with suitable arguments in debug mode
	Run Process
	...	${SYNCPED}
	...	-d
	...	-j	${file-config}
	...	-s	${file-input}
	...	-X	${file-output}
	...	-V	${level}
	...	../app.cpp
	...	${SYNCPED}
	...	stdout=${file-stdout}

Syncped Ex Mode
	[Documentation]	Runs syncped with suitable arguments in ex mode
	Run Process
	...	${SYNCPED}
	...	--ex
	...	-j	${file-config}
	...	-s	${file-input}
	...	-X	${file-output}
	...	-V	${level}
	...	${file-startup}
	...	stdout=${file-stdout}

*** Comments ***
Copyright: (c) 2020-2021 Anton van Wezenbeek
