#! /bin/csh -f

set resultDir = ~cs537-3/test/p1/results
set scriptDir = ~cs537-3/test/p1/scripts

if ($#argv == 1) then
    set list = $argv
else
    set list = "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24"
else
    echo "usage: run-tests.csh [whichTest]"
    exit 1
endif

@ PASS = 0
@ FAIL = 0
@ TAKEN = 0
foreach test ($list)
    set tmpFile = `mktemp`
    # echo $tmpFile

    # this is how you run the particular test
    (${scriptDir}/test${test}.csh > ${tmpFile}.out) >& ${tmpFile}.err
    set testStatus = $status

    # check for differences in output
    diff ${tmpFile}.out ${resultDir}/${test}.out >& ${tmpFile}.out.diff
    diff ${tmpFile}.err ${resultDir}/${test}.err >& ${tmpFile}.err.diff

    @ fail = 0

    # check if standard output, standard error are correct
    foreach type ("out" "err")
	set check = `stat -c %s ${tmpFile}.${type}.diff`
	if ($check != "0") then
	    echo "TEST ${test}: bad std${type} output"
	    echo "** Expected:"
	    cat ${resultDir}/${test}.${type}  | head -10
	    echo "** Got:"
	    cat ${tmpFile}.${type} | head -10
	    echo "** Diff file (which should be empty) instead contains:"
	    cat ${tmpFile}.${type}.diff | head -10
	    echo ""
	    @ fail = $fail + 1
	endif
    end

    # what return code should the program be giving us?
    set expectedStatus = `cat ${resultDir}/${test}.status`
    if ($testStatus != $expectedStatus) then
	echo "TEST ${test}: bad status"
	echo "** Expected return code: $expectedStatus"
	echo "** Got return code     : $testStatus"
	@ fail = $fail + 1
    endif

    if ($fail != 0) then
	echo "TEST ${test}: FAILED"
	@ FAIL = $FAIL + 1
    else
	echo "TEST ${test}: PASSED"
	@ PASS = $PASS + 1
    endif

    # clean up...
    rm -f in*.txt >& /dev/null
    rm -f out.txt >& /dev/null
    rm -f ${tmpFile}.* >& /dev/null
    echo ""
end

if ($#argv != 1) then
    echo "Summary:"
    echo "$PASS test(s) passed."
    @ grade = $PASS * 4
    echo "Total: $grade / 100"
endif


