*** Settings ***
Documentation	Testcases for syncped cmdline
Test Setup	Test Setup
Suite Setup	Syncped Suite Setup
Suite Teardown	Suite Teardown
Resource	keywords.resource


*** Test Cases ***
help
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

lexers
	[Documentation]	Check whether we have at least a rfw lexer
	${result}=	Run Process	${SYNCPED}	-L
	Should Contain	${result.stdout}	rfw


*** Comments ***
Copyright: (c) 2020-2022 Anton van Wezenbeek
