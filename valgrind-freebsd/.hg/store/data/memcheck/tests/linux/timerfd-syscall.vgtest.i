         {   z        ��������U� ������~NP�v            uprereq: [ "$(uname)" = "Linux" ] && awk 'END{exit("'$(uname -r)'" >= "2.6.25" ? 0 : 1)}' </dev/null
prog: timerfd-syscall
     {     C   B          ����X����hҥXu~5�n@���B�            uprereq: ../../../tests/os_test linux 2.6.25
prog: timerfd-syscall
