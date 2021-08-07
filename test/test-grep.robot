*** Settings ***
Documentation	Testcases for syncped grep
Test Setup	Test Setup
Suite Setup	Suite Setup
Suite Teardown	Suite Teardown
Resource	keywords.resource


*** Test Cases ***
HELP
	Input	:grep -h
	Syncped
	Output Contains	hidden
	Output Contains	recursive

TEXT	[Documentation]	Grep (without quit), and quit after some time
	${result}=	Run Process
	...	./count.sh

	Input Many	:grep rfw *.robot ./	1

	Syncped	5

	Output Contains	Found 3 matches in ${result.stdout} file(s)


*** Comments ***
Copyright: (c) 2020-2021 Anton van Wezenbeek
