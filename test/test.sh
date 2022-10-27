#!/usr/bin/bash

# @file test.sh
# @author Denis Horil (xhoril01@stud.fit.vutbr.cz)
# @brief Test script for feedreader
# @date 2022-10-23
#
# @copyright Copyright (c) 2022

export POSIXLY_CORRECT=yes

print_help()
{ 
    echo -e "\n============================================= HELP =============================================\n"
    echo
    echo -e "USAGE: ./test [-d <dir>] [-h || --help] --nodir \n"
    echo 
    echo -e "* Optional parameter '-d <directory>' -> will test every file in given directory\n"
    echo -e "* Optional parameter '-h || --help' -> will write out this 'Help statement'"
    echo -e "* Optional parameter '--nodir' -> will delete all test outputs in /test_outputs/ \n"
    echo
    echo -e "* If you want to use just one URL, write it inside text file and start script with '-d' parameter \n"
    echo -e "and specify path of its directory \n"
    echo -e "* If -d is not specified, /files will be used as main directory"
    echo
    echo -e "==================================================================================================\n"

}

RED='\033[1;31m'
GREEN='\033[1;32m'
BLUE='\033[1;34m'
NC='\033[0m'
SCRIPT='./feedreader'

line_args=$@
feedfile=0
url=0
files=()
directory="./test/files"
nodir=0

success=0
fail=0
count=0

# Reading args from cmd line
args_read()
{
    local prev_d=0

    for args in ${line_args[@]}
    do
        case $args in
            -h|--help) print_help
                        exit
                        ;;

            -d) prev_d=1
                ;;

            --nodir) nodir=1
                    ;;

            *)  if [ $prev_d -eq 1 ] 
                then
                    directory=($args)
                fi
                ;;
        esac
    done

    if [ $nodir -eq 1 ]
    then
        rm -r "./test/test_outputs"
        mkdir "./test/test_outputs"
    fi
}

# Reading feedfiles from given directory recursively
file_reader()
{
    local dir=$1
    local subdircount=$(find $dir -maxdepth 1 -type d | wc -l)
    local skip=1

    if [ $subdircount -gt 1 ]
    then
        for d in $(find $dir -maxdepth 1 -type d )
        do
            if [ $skip -eq 1 ]
            then
                skip=0
                continue
            fi
            file_reader $d
        done
    else
        for file in "$d"/*
        do
            files+=($file)
        done
    fi   
}

# Starting feedreader script with given feedfiles
read_start()
{
    for file in ${files[@]}
    do
        wrong_inputs $file
        correct_inputs $file
    done
}

# Wrong inputs
wrong_inputs()
{
    local file=$1
    local file_name
    local ret_code
    local tmp

    file_name="$(basename $file)"
    IFS='.' read file_name tmp <<< $file_name

    local NEWFILE="./test/test_outputs/${file_name}_w-Input.out"

    if [ -f $NEWFILE ]
    then
        rm $NEWFILE
    fi 

    touch $NEWFILE

    

    #############################################################
    # Test 1 - without everything
    echo -e "TEST_1: no parameter" >>$NEWFILE
    $SCRIPT 2>>$NEWFILE

    ret_code=$?
    if [ $ret_code -ne 1 ]
    then 
        echo -e "${RED}TEST_1_W ERROR:${NC}Expected return code is 1 but is ${ret_code}" 1>&2  
        let "fail+=1"
    else
        let "success+=1"
    fi
    let "count+=1"

    echo "=====================================">>$NEWFILE

    #############################################################
    # Test 2 - with bad parameter
    echo -e "TEST_2: Bad parameters" >>$NEWFILE
    $SCRIPT random1 random2 2>>$NEWFILE

    ret_code=$?
    if [ $ret_code -ne 1 ]
    then 
        echo -e "${RED}TEST_2_W ERROR:${NC}Expected return code is 1 but is ${ret_code}" 1>&2
        let "fail+=1"
    else
        let "success+=1"
    fi
    let "count+=1"

    echo "=====================================">>$NEWFILE

    #############################################################
    # Test 3 - -f and URL both
    echo -e "TEST_3: Both -f an URL" >>$NEWFILE
    $SCRIPT -f $file https://xkcd.com/atom.xml 2>>$NEWFILE

    ret_code=$?
    if [ $ret_code -ne 1 ]
    then 
        echo -e "${RED}TEST_3_W ERROR:${NC}Expected return code is 1 but is ${ret_code}" 1>&2
        let "fail+=1"
    else
        let "success+=1"
    fi
    let "count+=1"

    echo "=====================================">>$NEWFILE

    #############################################################
    # Test 4 - -f and bad parameter
    echo -e "TEST_4: -f and 2 bad parameters" >>$NEWFILE
    $SCRIPT -f $file random1 random2 2>>$NEWFILE

    ret_code=$?
    if [ $ret_code -ne 1 ]
    then 
        echo -e "${RED}TEST_4_W ERROR:${NC}Expected return code is 1 but is ${ret_code}" 1>&2
        let "fail+=1"
    else
        let "success+=1"
    fi
    let "count+=1"

    echo "=====================================">>$NEWFILE

    #############################################################
    # Test 5 - -URL and bad parameter
    echo -e "TEST_5: URL and bad parameter" >>$NEWFILE
    $SCRIPT https://xkcd.com/atom.xml random 2>>$NEWFILE

    ret_code=$?
    if [ $ret_code -ne 1 ]
    then 
        echo -e "${RED}TEST_5_W ERROR:${NC}Expected return code is 1 but is ${ret_code}" 1>&2
        let "fail+=1"
    else
        let "success+=1"
    fi
    let "count+=1"

    echo "=====================================">>$NEWFILE

    #############################################################
    #Test 6 - Unknown flag
    echo -e "TEST_6: Unknown flag" >>$NEWFILE
    $SCRIPT -f $file -b 2>>$NEWFILE

    ret_code=$?
    if [ $ret_code -ne 1 ]
    then 
        echo -e "${RED}TEST_6_W ERROR:${NC}Expected return code is 1 but is ${ret_code}" 1>&2
        let "fail+=1"
    else
        let "success+=1"
    fi
    let "count+=1"

    echo "=====================================">>$NEWFILE

    #############################################################
    # Test 7 - Unknown flag among known ones
    echo -e "TEST_7: Unknown flag among known ones" >>$NEWFILE
    $SCRIPT -f $file -T -u -d -a 2>>$NEWFILE

    ret_code=$?
    if [ $ret_code -ne 1 ]
    then 
        echo -e "${RED}TEST_7_W ERROR:${NC}Expected return code is 1 but is ${ret_code}" 1>&2
        let "fail+=1"
    else
        let "success+=1"
    fi
    let "count+=1"

    echo "=====================================">>$NEWFILE

    #############################################################
    # Test 8 - Required arguments with flags
    echo -e "TEST_8: Required argument with -c" >>$NEWFILE
    $SCRIPT -f $file -c 2>>$NEWFILE

    ret_code=$?
    if [ $ret_code -ne 1 ]
    then 
        echo -e "${RED}TEST_8_W ERROR:${NC}Expected return code is 1 but is ${ret_code}" 1>&2
        let "fail+=1"
    else
        let "success+=1"
    fi
    let "count+=1"

    echo "=====================================">>$NEWFILE

    #############################################################
    # Test 9 - Required arguments with flags
    echo -e "TEST_9:Required argument with -f" >>$NEWFILE
    $SCRIPT -f $file -f 2>>$NEWFILE

    ret_code=$?
    if [ $ret_code -ne 1 ]
    then 
        echo -e "${RED}TEST_9_W ERROR:${NC}Expected return code is 1 but is ${ret_code}" 1>&2
        let "fail+=1"
    else
        let "success+=1"
    fi
    let "count+=1"

    echo "=====================================">>$NEWFILE

    #############################################################
    # Test 10 - Required arguments with flags
    echo -e "TEST_10: Required argument with -C" >>$NEWFILE
    $SCRIPT -f $file -C 2>>$NEWFILE

    ret_code=$?
    if [ $ret_code -ne 1 ]
    then 
        echo -e "${RED}TEST_10_W ERROR:${NC}Expected return code is 1 but is ${ret_code}" 1>&2
        let "fail+=1"
    else
        let "success+=1"
    fi
    let "count+=1"

    echo "=====================================">>$NEWFILE

    #############################################################
    # Test 11 - Unsupported url scheme
    echo -e "TEST_11: Unsupported url scheme" >>$NEWFILE
    $SCRIPT ftp://www.theregister.co.uk/data_centre/headlines.atom 2>>$NEWFILE

    ret_code=$?
    if [ $ret_code -ne 1 ]
    then 
        echo -e "${RED}TEST_11_W ERROR:${NC}Expected return code is 1 but is ${ret_code}" 1>&2
        let "fail+=1"
    else
        let "success+=1"
    fi
    let "count+=1"

    echo "=====================================">>$NEWFILE

    #############################################################
}

# Correct inputs
correct_inputs()
{
    local file=$1
    local file_name
    local file_name_txt
    local ret_code
    local tmp

    file_name_txt="$(basename $file)"
    IFS='.' read file_name tmp <<< $file_name_txt

    local NEWFILE="./test/test_outputs/${file_name}_c-Input.out"
    local ERRFILE="./test/test_outputs/${file_name}_c-Input.err"

    if [ -f $NEWFILE ]
    then
        rm $NEWFILE
    fi 

    touch $NEWFILE

    if [ -f $ERRFILE ]
    then
        rm $ERRFILE
    fi 

    touch $ERRFILE

    #############################################################
    # Test 1 - Help statement
    echo -e "TEST_1: Help statement" >>$NEWFILE
    echo -e "TEST_1: Help statement" >>$ERRFILE
    $SCRIPT -f $file -h 1>>$NEWFILE 2>>$ERRFILE

    ret_code=$?
    if [ $ret_code -ne 0 ]
    then 
        echo -e "${RED}TEST_1_C ERROR in file '${file_name_txt}':${NC}Expected return code is 0 but is ${ret_code}." 1>&2
        echo -e "${BLUE}Check the .err file for error statement.${NC}"  1>&2
        echo -e "" 1>&2 
        let "fail+=1"
    else
        let "success+=1"
    fi
    let "count+=1"

    echo "=====================================">>$NEWFILE
    echo "=====================================">>$ERRFILE
    echo "">>$NEWFILE
    echo "">>$ERRFILE

    ############################################################
    # Test 2 - Help statement with other flags
    echo -e "TEST_2: Help statement with other flags" >>$NEWFILE
    echo -e "TEST_2: Help statement with other flags" >>$ERRFILE
    $SCRIPT -f $file -T -u -a -h 1>>$NEWFILE 2>>$ERRFILE

    ret_code=$?
    if [ $ret_code -ne 0 ]
    then 
        echo -e "${RED}TEST_2_C ERROR in file '${file_name_txt}':${NC}Expected return code is 0 but is ${ret_code}." 1>&2
        echo -e "${BLUE}Check the .err file for error statement.${NC}"  1>&2
        echo -e "" 1>&2
        let "fail+=1"
    else
        let "success+=1"
    fi
    let "count+=1"

    echo "=====================================">>$NEWFILE
    echo "=====================================">>$ERRFILE
    echo "">>$NEWFILE
    echo "">>$ERRFILE

    ############################################################
    # Test 3 - No flags
    echo -e "TEST_3: No flags" >>$NEWFILE
    echo -e "TEST_3: No flags" >>$ERRFILE
    $SCRIPT -f $file 1>>$NEWFILE 2>>$ERRFILE

    ret_code=$?
    if [ $ret_code -ne 0 ]
    then 
        echo -e "${RED}TEST_3_C ERROR in file '${file_name_txt}':${NC}Expected return code is 0 but is ${ret_code}." 1>&2
        echo -e "${BLUE}Check the .err file for error statement.${NC}"  1>&2
        echo -e "" 1>&2
        let "fail+=1"
    else
        let "success+=1"
    fi
    let "count+=1"

    echo "=====================================">>$NEWFILE
    echo "=====================================">>$ERRFILE
    echo "">>$NEWFILE
    echo "">>$ERRFILE

    ############################################################
    # Test 4 - -T
    echo -e "TEST_4: -T flag" >>$NEWFILE
    echo -e "TEST_4: -T flag" >>$ERRFILE
    $SCRIPT -f $file -T 1>>$NEWFILE 2>>$ERRFILE

    ret_code=$?
    if [ $ret_code -ne 0 ]
    then 
        echo -e "${RED}TEST_4_C ERROR in file '${file_name_txt}':${NC}Expected return code is 0 but is ${ret_code}." 1>&2
        echo -e "${BLUE}Check the .err file for error statement.${NC}"  1>&2
        echo -e "" 1>&2 
        let "fail+=1"
    else
        let "success+=1"
    fi
    let "count+=1"

    echo "=====================================">>$NEWFILE
    echo "=====================================">>$ERRFILE
    echo "">>$NEWFILE
    echo "">>$ERRFILE

    ############################################################
    # Test 5 - -u
    echo -e "TEST_5: -u flag" >>$NEWFILE
    echo -e "TEST_5: -u flag" >>$ERRFILE
    $SCRIPT -f $file -u 1>>$NEWFILE 2>>$ERRFILE

    ret_code=$?
    if [ $ret_code -ne 0 ]
    then 
        echo -e "${RED}TEST_5_C ERROR in file '${file_name_txt}':${NC}Expected return code is 0 but is ${ret_code}." 1>&2
        echo -e "${BLUE}Check the .err file for error statement.${NC}"  1>&2
        echo -e "" 1>&2
        let "fail+=1"
    else
        let "success+=1"
    fi
    let "count+=1"

    echo "=====================================">>$NEWFILE
    echo "=====================================">>$ERRFILE
    echo "">>$NEWFILE
    echo "">>$ERRFILE

    ############################################################
    # Test 6 - -a
    echo -e "TEST_6: -a flag" >>$NEWFILE
    echo -e "TEST_6: -a flag" >>$ERRFILE
    $SCRIPT -f $file -a 1>>$NEWFILE 2>>$ERRFILE

    ret_code=$?
    if [ $ret_code -ne 0 ]
    then 
        echo -e "${RED}TEST_6_C ERROR in file '${file_name_txt}':${NC}Expected return code is 0 but is ${ret_code}." 1>&2
        echo -e "${BLUE}Check the .err file for error statement.${NC}"  1>&2
        echo -e "" 1>&2 
        let "fail+=1"
    else
        let "success+=1"
    fi
    let "count+=1"

    echo "=====================================">>$NEWFILE
    echo "=====================================">>$ERRFILE
    echo "">>$NEWFILE
    echo "">>$ERRFILE

    ############################################################
    # Test 7 - -a -u
    echo -e "TEST_7: -a -u flag" >>$NEWFILE
    echo -e "TEST_7: -a -u flag" >>$ERRFILE
    $SCRIPT -f $file -a -u 1>>$NEWFILE 2>>$ERRFILE

    ret_code=$?
    if [ $ret_code -ne 0 ]
    then 
        echo -e "${RED}TEST_7_C ERROR in file '${file_name_txt}':${NC}Expected return code is 0 but is ${ret_code}." 1>&2
        echo -e "${BLUE}Check the .err file for error statement.${NC}"  1>&2
        echo -e "" 1>&2
        let "fail+=1"
    else
        let "success+=1"
    fi
    let "count+=1"

    echo "=====================================">>$NEWFILE
    echo "=====================================">>$ERRFILE
    echo "">>$NEWFILE
    echo "">>$ERRFILE

    ############################################################
    # Test 8 - -a -T
    echo -e "TEST_8: -a -T flag" >>$NEWFILE
    echo -e "TEST_8: -a -T flag" >>$ERRFILE
    $SCRIPT -f $file -a -T 1>>$NEWFILE 2>>$ERRFILE

    ret_code=$?
    if [ $ret_code -ne 0 ]
    then 
        echo -e "${RED}TEST_8_C ERROR in file '${file_name_txt}':${NC}Expected return code is 0 but is ${ret_code}." 1>&2
        echo -e "${BLUE}Check the .err file for error statement.${NC}"  1>&2
        echo -e "" 1>&2
        let "fail+=1"
    else
        let "success+=1"
    fi
    let "count+=1"

    echo "=====================================">>$NEWFILE
    echo "=====================================">>$ERRFILE
    echo "">>$NEWFILE
    echo "">>$ERRFILE

    ############################################################
    # Test 9 - -u -T
    echo -e "TEST_9: -u -T flag" >>$NEWFILE
    echo -e "TEST_9: -u -T flag" >>$ERRFILE
    $SCRIPT -f $file -u -T 1>>$NEWFILE 2>>$ERRFILE

    ret_code=$?
    if [ $ret_code -ne 0 ]
    then 
        echo -e "${RED}TEST_9_C ERROR in file '${file_name_txt}':${NC}Expected return code is 0 but is ${ret_code}." 1>&2
        echo -e "${BLUE}Check the .err file for error statement.${NC}"  1>&2
        echo -e "" 1>&2
        let "fail+=1"
    else
        let "success+=1"
    fi
    let "count+=1"

    echo "=====================================">>$NEWFILE
    echo "=====================================">>$ERRFILE
    echo "">>$NEWFILE
    echo "">>$ERRFILE

    ############################################################
    # Test 10 - -u -T -a
    echo -e "TEST_10: -u -T -a flag" >>$NEWFILE
    echo -e "TEST_10: -u -T -a flag" >>$ERRFILE
    $SCRIPT -f $file -u -T -a 1>>$NEWFILE 2>>$ERRFILE

    ret_code=$?
    if [ $ret_code -ne 0 ]
    then 
        echo -e "${RED}TEST_10_C ERROR in file '${file_name_txt}':${NC}Expected return code is 0 but is ${ret_code}." 1>&2
        echo -e "${BLUE}Check the .err file for error statement.${NC}"  1>&2 
        echo -e "" 1>&2
        let "fail+=1"
    else
        let "success+=1"
    fi
    let "count+=1"

    echo "=====================================">>$NEWFILE
    echo "=====================================">>$ERRFILE
    echo "">>$NEWFILE
    echo "">>$ERRFILE

    ############################################################
    # Test 11 - Wrong <certaddr> with -C
    echo -e "TEST_11: Wrong <certaddr> with -C" >>$NEWFILE
    echo -e "TEST_11: Wrong <certaddr> with -C" >>$ERRFILE
    $SCRIPT -f $file -C usr/dev/null 1>>$NEWFILE 2>>$ERRFILE

    ret_code=$?
    if [ $ret_code -ne 0 ]
    then 
        echo -e "${RED}TEST_11_C ERROR:${NC}Expected return code is 0 but is ${ret_code}" 1>&2
        let "fail+=1"
    else
        let "success+=1"
    fi
    let "count+=1"

    echo "=====================================">>$NEWFILE
    echo "=====================================">>$ERRFILE
    echo "">>$NEWFILE
    echo "">>$ERRFILE

    #############################################################

}

# -------------------- MAIN ----------------------- #

echo -e "\nStarting tests ..."
args_read

if [ ! -d "./test/test_outputs" ]
then
    mkdir "./test/test_outputs"
fi

file_reader $directory
read_start

echo -e "============== SUMMARY ==============="
percentage=$(( success / count ))
percentage=$(( percentage * 100 ))

echo -e "${RED}FAILED: ${NC}$fail"
echo -e "${GREEN}SUCCEEDED: ${NC}$success"
echo
echo -e "${BLUE}PERCENTAGE: ${NC}$percentage %"

echo -e "\n* Check file 'test_outputs' for more"
echo -e "  details about 'feedreader' outputs\n"

echo -e "======================================"
# ----------------- END OF MAIN ------------------- #