*** Comments ***
Copyright: (c) 2020-2023 Anton van Wezenbeek

*** Settings ***
Documentation	Keywords for syncped
Library	OperatingSystem
Library	Process
Resource	/usr/local/lib/wex-keywords.resource


*** Variables ***
${SYNCPED}	${EMPTY}

${file-config}	test.json


*** Keywords ***
Find Syncped
	${result}=	Run Process
	...	find	../
	...	-name	syncped
	...	-type	f
	Set Suite Variable	${SYNCPED}	${result.stdout}

Syncped Suite Setup
	Find Syncped
	Variable Should Exist	${SYNCPED}

Syncped
	[Documentation]	Runs syncped with suitable arguments
	[Arguments]	${quit}=0
	Run Process
	...	${SYNCPED}
	...	-j	${file-config}
	...	-q	${quit}
	...	-s	${file-input}
	...	-V	${level}
	...	-X	${file-output}
	...	${file-startup}
	...	stdout=${file-stdout}

Syncped Debug
	[Documentation]	Runs syncped with suitable arguments in debug mode
	[Arguments]	${quit}=0
	Run Process
	...	${SYNCPED}
	...	-d
	...	-j	${file-config}
	...	-q	${quit}
	...	-s	${file-input}
	...	-V	${level}
	...	-X	${file-output}
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
