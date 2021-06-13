*** Settings ***
Documentation	Testcases for syncped
Test Setup	Test Setup
Suite Setup	Suite Setup
Suite Teardown	Suite Teardown
Library	OperatingSystem
Library	Process


*** Variables ***
# Normally syncped exits after each test, override this
# variable on the commandline using '-v QUIT:0' to remain active
${QUIT}	1

# Normally syncped runs in quiet mode, only errors are logged,
# override this variable using '-v SEVERITY-LEVEL:0' to run in verbose mode
# (only shown in the log file).
${SEVERITY-LEVEL}	4

${FILE-CONFIG}	test.json
${FILE-CONTENTS}	contents.txt
${FILE-INPUT}	input.txt
${FILE-OUTPUT}	output.txt
${FILE-STARTUP}	empty.txt

${SYNCPED}


*** Test Cases ***
TC-HELP
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

TC-LEXERS
	[Documentation]	Check whether we have at least a rfw lexer
	${result}=	Run Process	${SYNCPED}	-L
	Should Contain	${result.stdout}	rfw

TC-EMPTY
	Input	:1000
	...	:.=
	Syncped
	Output Contains	1

# ex tests

TC-EX-EDIT
	Input	:e other.txt
	...	:f
	Syncped
	# the :e command is handled by event, so other.txt not yet active
	Output Contains	${FILE-STARTUP}

TC-EX-INFO
	Input	:a|line has text
	...	:f
	Syncped
	Output Contains	${FILE-STARTUP}
	Output Contains	1
	Output Contains	%
	Output Contains	level

TC-EX-MDI
	Input	:a|line has text
	...	:e other.txt
	...	:e more.txt
	# We cannot test mdi, because of events.
	Syncped

TC-EX-PROCESS
	Input	:!pwd
	Syncped
	Output Contains	build

TC-EX-SET
	Input	:set all *
	Syncped
	Output Contains	ts=

TC-EX-SET-BOOL
	Input	:set nosws *
	...	:set sws ? *
	Syncped
	Output Contains	nosws

TC-EX-SET-INFO
	Input	:set ts ? *
	Syncped
	Output Contains	ts=

TC-EX-SET-VERBOSITY
	Input	:set ve?
	Syncped
	Output Contains	ve=${SEVERITY-LEVEL}

TC-EX-SUBSTITUTE
	Input	:a|line has text
	...	:a|line has a tiger
	...	:a|line has simon and simon and garfunkel
	...	:%s/simon/nick/
	Syncped
	Output Contains	1
	Output Contains	simon

TC-EX-SUBSTITUTE-GLOBAL
	Input	:a|line has text
	...	:a|line has a tiger
	...	:a|line has simon and simon and garfunkel
	...	:%s/simon/nick/g
	Syncped
	Output Contains	2
	Output Contains	simon
	Contents Does Not Contain	simon

# vi tests

TC-VI-CALCULATE
	Input	:a|x
	...	=9+9+9+9
	Syncped
	Output Contains	36

TC-VI-DELETE
	Input Many	:a|line	100
	Input	:1
	...	59dd
	Syncped
	Output Contains	59
	Output Contains	fewer

TC-VI-DELETE-D
	Input	:a|line has some text
	...	:1
	...	ww
	...	D
	Syncped
	Contents Does Not Contain	some text

TC-VI-FIND-NOT
	Input	:a|x
	...	:a|y
	...	:a|z
	...	/zz
	...	:.=
	Syncped
	Output Contains	4

TC-VI-FIND-OK
	Input	:a|x
	...	:a|y
	...	:a|z
	...	/z
	...	:.=
	Syncped
	Output Contains	3

TC-VI-INFO
	Input	:a|line has text
	...	
	Syncped
	Output Contains	1
	Output Contains	%
	Output Contains	level

TC-VI-MACRO
	Input	@Template-test@
	Syncped
	Contents Does Not Contain	@Created@
	Contents Does Not Contain	@Date@
	Contents Does Not Contain	'Date'
	Contents Does Not Contain	@Datetime@
	Contents Does Not Contain	@Process@
	Contents Does Not Contain	@Year@

TC-VI-MARKER
	Input Many	:a|line has text	50
	Input	:10
	...	mx
	...	:1
	...	'x
	...	:.=
	Syncped
	Output Contains	10

TC-VI-MODE-BLOCK
	Input Many	:a|line has text	50
	Input	:1
	...	w
	...	K
	...	10j
	...	w
	...	d
	Syncped
	Output Does Not Contain	10

TC-VI-MODE-EX
	Input No Write	:vi
	Syncped Ex Mode

TC-VI-MODE-INSERT
	Input	:a|one line
	...	ijjjjjjj
	Syncped
	Contents Contains	jjjjjjj

TC-VI-MODE-VISUAL
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

TC-VI-NAVIGATE
	Input Many	:a|line has text	50
	Input	:1
	...	jjjjjjj
	...	:.=
	Syncped
	Output Contains	8

TC-VI-YANK
	Input Many	:a|line	100
	Input	:1
	...	59yy
	Syncped
	Output Contains	59
	Output Contains	yanked


*** Keywords ***
Suite Setup
	Find Syncped
	Variable Should Exist	${SYNCPED}

Suite Teardown
	Remove File	${file-contents}
	Remove File	${file-input}
	Remove File	${file-output}

Test Setup
	Create File	${FILE-INPUT}
	Create File	${FILE-OUTPUT}

Input
	[Arguments]	@{text}
	FOR	${cmd}	IN	@{text}
		Append To File	${FILE-INPUT}	${cmd}
		Append To File	${FILE-INPUT}	\n
	END

	Append To File	${FILE-INPUT}	:w ${FILE-CONTENTS}\n
	IF	${QUIT} == 1
		Append To File	${FILE-INPUT}	:q!
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

Find Syncped
	${result}=	Run Process
	...	find	./
	...	-name	syncped
	...	-type	f
	Set Suite Variable	${SYNCPED}	${result.stdout}

Syncped
	[Documentation]	Runs syncped with suitable arguments
	Run Process
	...	${SYNCPED}
	...	-j	${file-config}
	...	-s	${file-input}
	...	-X	${file-output}
	...	-V	${severity-level}
	...	${file-startup}

Syncped Ex Mode
	[Documentation]	Runs syncped with suitable arguments in ex mode
	Run Process
	...	${SYNCPED}
	...	--ex
	...	-j	${file-config}
	...	-s	${file-input}
	...	-X	${file-output}
	...	-V	${severity-level}
	...	${file-startup}

Contents Contains
	[Arguments]	${text}
	${result}=	Get File	${file-contents}
	Should Contain	${result}	${text}

Contents Does Not Contain
	[Arguments]	${text}
	${result}=	Get File	${FILE-CONTENTS}
	Should Not Contain	${result}	${text}

Output Contains
	[Arguments]	${text}
	${result}=	Get File	${FILE-OUTPUT}
	Should Contain	${result}	${text}

Output Does Not Contain
	[Arguments]	${text}
	${result}=	Get File	${file-output}
	Should Not Contain	${result}	${text}


*** Comments ***
Copyright: (c) 2020-2021 Anton van Wezenbeek
