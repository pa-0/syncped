*** Settings ***
Documentation	Keywords for syncped
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
	...	find	../
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
