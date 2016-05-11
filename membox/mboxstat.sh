#!/bin/bash 

if [ ! -e $1 ]  ; then
	printf impossibile stampare $1 dato che non esiste
fi


#  # tail -n -1 $1 | read s			 
#  # perchè così non funziona? non dovrebe reindirizzare l'outpput del tail sull'input del read?
																	
tail -n -1 $1>3
read s<3

s=${s#*-}

i=0

for n in $s ; do
	if [ $i -eq 1 ]; then  printf "n. di PUT_OP ricevute = %s\n" "$n" ;fi
	if [ $i -eq 2 ] ; then  printf "n. di PUT_OP fallite = %s\n" "$n" ;fi
	if [ $i -eq 3 ] ; then  printf "n. di UPDATE_OP ricevute = %s\n" "$n" ;fi
	if [ $i -eq 4 ] ; then  printf "n. di UPDATE_OP fallite = %s\n" "$n" ;fi
	if [ $i -eq 5 ] ; then  printf "n. di GET_OP ricevute = %s\n" "$n" ;fi
	if [ $i -eq 6 ] ; then  printf "n. di GET_OP fallite = %s\n" "$n" ;fi
	if [ $i -eq 7 ] ; then  printf "n. di REMOVE_OP ricevute = %s\n" "$n" ;fi
	if [ $i -eq 8 ] ; then  printf "n. di REMOVE_OP fallite = %s\n" "$n" ;fi
	if [ $i -eq 9 ] ; then  printf "n. di LOCK_OP ricevute = %s\n" "$n" ;fi
	if [ $i -eq 10 ] ; then  printf "n. di LOCK_OP fallite = %s\n" "$n" ;fi
	if [ $i -eq 11 ] ; then  printf "n. massimo di connessioni concorrenti raggiunte = %s\n" "$n" ;fi
	if [ $i -eq 12 ] ; then  printf "n. massimo di oggetti memorizzati raggiunto = %s\n" "$n" ;fi
	if [ $i -eq 13 ] ; then  printf "n. corrente di oggetti memorizzati = %s\n" "$n" ;fi
	if [ $i -eq 14 ] ; then  printf "size massima in bytes raggiunta = %s\n" "$n" ;fi
	if [ $i -eq 15 ] ; then  printf "size in bytes corrente = %s\n" "$n" ;fi
	let i=i+1
done

echo 

l=${@%$1}" "$1

pp=0;uu=0;gg=0;rr=0;cc=0;ss=0;oo=0;mm=0;hh=0

while getopts ":p:s" ops $l; do
  case $ops in
        p)
            # echo  opzione p
			pp=1
            ;;
        u)
            # echo  opzione u
            uu=1
            ;;
        g)
            # echo  opzione g
            gg=1
            ;;
        r)
            # echo  opzione r
            rr=1
            ;;
        c)
            # echo  opzione c
            cc=1
            ;;
        s)
            # echo  opzione s
            ss=1
            ;;
        o)
            # echo  opzione o
            oo=1
            ;;
        m)
            # echo  opzione m
            mm=1
            ;;  
        help)
            # echo  opzione help
            hh=1
            ;;  
  esac
done

while read data trat a b c d e f g h i l m n o p q ; do

	printf "\nle statistice del timestamp all'orario %s erano\n" "$data" 

	if [ $pp -eq 1 ]; then  
		printf "n. di PUT_OP ricevute = %s\n" "$a" 
		printf "n. di PUT_OP fallite = %s\n" "$b" 
	fi

	if [ $uu -eq 1 ]; then  
		printf "n. di UPDATE_OP ricevute = %s\n" "$c" 
		printf "n. di UPDATE_OP fallite = %s\n" "$d" 
	fi

	if [ $gg -eq  1 ]; then  
		printf "n. di GET_OP ricevute = %s\n" "$e" 
		printf "n. di GET_OP fallite = %s\n" "$f" 
	fi

	if [ $rr -eq  1 ]; then  
		printf "n. di REMOVE_OP ricevute = %s\n" "$n" 
		printf "n. di REMOVE_OP fallite = %s\n" "$n" 
	fi

	if [ $oo -eq  1 ]; then  
		printf "n. corrente di oggetti memorizzati = %s\n" "$n" 
	fi

	if [ $mm -eq  1 ]; then  
		printf "n. massimo di connessioni concorrenti raggiunte = %s\n" "$n"
		printf "n. massimo di oggetti memorizzati raggiunto = %s\n" "$n"
		printf "size massima in bytes raggiunta = %s\n" "$n" 
	fi


done < $1

echo