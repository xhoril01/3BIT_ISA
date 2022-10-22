#!usr/bin/bash

export POSIXLY_CORRECT=yes


print_help()
{ 
    echo -e "\n============================================= HELP =============================================\n"
    echo
    echo -e "USAGE: ./test [-f <feedfile>] [-h || --help]\n"
    echo 
    echo -e "* Optional parameter '-f <feedfile>' -> will test script 'feedreader' with feeds in feedfile\n"
    echo -e "* Optional parameter '-h || --help' -> will write out this 'Help statement'"
    echo
    echo -e "If you want to use just one URL, write it inside text file and start script with '-f' parameter.\n"
    echo
    echo -e "==================================================================================================\n"

}

RED='\033[1;31m'
GREEN='\033[1;32m'
NC='\033[0m'

line_args=$@
feedfile=0
url=0
files=()

# Reading args from cmd line
args_read()
{
    local is_f=0
    local is_u=0

    for args in ${line_args[@]}
    do
        case $args in
            -h||--help) print_help
                        exit
                        ;;

            -f) is_f=1
                exit
                ;;

            -u) is_u=1
                exit
                ;;
            
            *)  if [is_f -eq 1]
                then
                    feedfile=$args
                    if [ ${#feedfile[@]} -eq 0]
                    then
                        echo -e "${RED}E:${NC} Missing <feedfile> with '-f'. Type -h or --help for help\n"
                        exit -1
                    fi
                elif [is_u -eq 1]
                then
                    url=$args
                    if [ ${#url[@]} -eq 0]
                    then
                        echo -e "${RED}E:${NC} Missing <URL> with '-u'. Type -h or --help for help\n"
                    fi
                fi
        esac
    done
}

# TODO
# Reading feedfiles from ./test/ directory
file_reader()
{

}

# TODO
# Starting feedreader script with given feedfiles
read_start()
{

}

# -------------------- MAIN ----------------------- #

args_read()
file_reader()
read_start()

# ----------------- END OF MAIN ------------------- #