*** Settings ***
Documentation	Testcases for syncped vi
Test Setup	Test Setup
Suite Setup	Syncped Suite Setup
Suite Teardown	Suite Teardown
Resource	keywords.resource


*** Test Cases ***
debug	[Documentation]	set a breakpoint, and give time to process it
	Input Many	:23	1
	Input Many	:de b	1
	Syncped Debug	20000
	Output Contains	lldb
	Output Contains	Breakpoint

mode-ex
	Input No Write	:vi
	Syncped Ex Mode


*** Comments ***
Copyright: (c) 2020-2022 Anton van Wezenbeek
