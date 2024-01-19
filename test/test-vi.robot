*** Comments ***
Copyright: (c) 2020-2023 Anton van Wezenbeek

*** Settings ***
Documentation	Testcases for syncped vi
Test Setup	Test Setup
Suite Setup	Syncped Suite Setup
Suite Teardown	Suite Teardown
Resource	keywords.resource


*** Test Cases ***
debug	[Documentation]	set a breakpoint, and give time to process it
	${platform}=	Evaluate	sys.platform	sys
	Input Many	:23	1
	Input Many	:de b	1
	Syncped Debug	20000
	IF	"${platform}"=="darwin"
		Output Contains	lldb
	ELSE
		Output Contains	gdb
	END
	Output Contains	Breakpoint

mode-ex
	Input No Write	:vi
	Syncped Ex Mode
