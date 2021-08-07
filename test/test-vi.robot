*** Settings ***
Documentation	Testcases for syncped vi
Test Setup	Test Setup
Suite Setup	Suite Setup
Suite Teardown	Suite Teardown
Resource	keywords.resource


*** Test Cases ***
CALCULATE
	Input	=9+9+9+9-2+(3*3)
	...	=2<4
	...	=128/2
	...	=127&7
	Syncped
	Output Contains	43
	Output Contains	32
	Output Contains	64
	Output Contains	7

DEBUG	[Documentation]	Set a breakpoint, and give time to process it
	Input Many	:23	1
	Input Many	:de b	1
	Syncped Debug	20
	Output Contains	lldb
	Output Contains	Breakpoint

DELETE
	Input Many	:a|line	100
	Input	:1
	...	59dd
	Syncped
	Output Contains	59
	Output Contains	fewer

DELETE-D
	Input	:a|line has some text
	...	:1
	...	ww
	...	D
	Syncped
	Contents Does Not Contain	some text

FIND-NOT
	Input	:a|x
	...	:a|y
	...	:a|z
	...	/zz
	...	:.=
	Syncped
	Output Contains	4

FIND-OK
	Input	:a|x
	...	:a|y
	...	:a|z
	...	/z
	...	:.=
	Syncped
	Output Contains	3

INFO
	Input	:a|line has text
	...	
	Syncped
	Output Contains	1
	Output Contains	%
	Output Contains	level

MACRO
	Input	@Template-test@
	Syncped
	Contents Does Not Contain	@Created@
	Contents Does Not Contain	@Date@
	Contents Does Not Contain	'Date'
	Contents Does Not Contain	@Datetime@
	Contents Does Not Contain	@Process@
	Contents Does Not Contain	@Year@

MACRO-RECORD
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

MARKER
	Input Many	:a|line has text	50
	Input	:10
	...	mx
	...	:1
	...	'x
	...	:.=
	Syncped
	Output Contains	10

MODE-BLOCK
	Input Many	:a|line has text	50
	Input	:1
	...	w
	...	K
	...	10j
	...	w
	...	d
	Syncped
	Output Does Not Contain	10

MODE-EX
	Input No Write	:vi
	Syncped Ex Mode

MODE-INSERT
	Input	:a|one line
	...	ijjjjjjj
	Syncped
	Contents Contains	jjjjjjj

MODE-VISUAL
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

NAVIGATE
	Input Many	:a|line has text	50
	Input	:1
	...	jjjjjjj
	...	:.=
	Syncped
	Output Contains	8

YANK
	Input Many	:a|line	100
	Input	:1
	...	59yy
	Syncped
	Output Contains	59
	Output Contains	yanked

YANK-REGISTER
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


*** Comments ***
Copyright: (c) 2020-2021 Anton van Wezenbeek
