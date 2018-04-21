#!/bin/bash 

#controllo che il file passato come parametro esita
if [ ! -e $1 ]  ; then																				
	printf impossibile utilizzare $1 dato che non esiste
fi



echo 																									
	if [ $2 ] ; then

	#configuro le opzioni
		PARSED_OPTIONS=$(getopt -n "$0"  -o psugrcsom --long "help per"  -- "$@") 						

	# se ci sono degli argomenti errati non faccio niente
	if [ $? -ne 0 ];
	then
		echo alcuni degli argomenti passati sono errati
	else
			eval set -- "$PARSED_OPTIONS"

		#inizzializzo a zero le variabili che mi serviranno per le varie opsioneioni
		pp=0;uu=0;gg=0;rr=0;ccc=0;ss=0;oo=0;mm=0;hh=0													
		perr=0;

		#nel caso un opsione sia tra i parametri setto a uno la variabile corelata, che succesivamente servirà per attuare l'opzione
			while true;																					
			do
			  case $1 in 
			        -p)
			             #echo  opzione p
						pp=1
			                  shift;;
			        -u)
			            # echo  opzione u
			            uu=1
			                  shift;;
			        -g)
			            # echo  opzione g
			            gg=1
			                  shift;;
			        -r)
			            # echo  opzione r
			            rr=1
			                  shift;;
			        -c)
			            # echo  opzione c
			            ccc=1
			                  shift;;
			        -s)
			            # echo  opzione s
			            ss=1
			                  shift;;
			        -o)
			            # echo  opzione o
			            oo=1
			                  shift;;
			        -m)
			            # echo  opzione m
			            mm=1
			                  shift;;
			        --help)
			             #echo  opzione aiuto
			            hh=1
			                  shift;;
			        --per)
			             #echo  opzione percentuale
						perr=1			            
			                  shift;;		        
		   			 --)
		 			     shift
		  			    break;;
			  esac
			done
		#setto a zero i vari massimi
			max_a_nc=0																					
			max_a_s=0
			max_a_ng=0


		#scorro l'intero file delle statistiche a "righe", e per ogni ciclo del while che corrisponde a una riga, nel caso siano settati a 1 le variabili corrispondenti a un opzione, eseguo l'opzione
		if	[ $pp -eq 1 ] || [ $uu -eq 1 ] || [ $gg -eq  1 ] || [ $ccc -eq  1 ] || [ $ss -eq  1 ] || [ $oo -eq  1 ]  ; then
			while read data trattino put_o put_f up_o up_f get_o get_f rem_o rem_f lk_o lk_f nc siz max_s n_ogg max_ng ; do

				printf "\nle statistice del timestamp all'orario %s erano\n" "$data" 

				if [ $pp -eq 1 ]; then  
					printf "n. di PUT_OP ricevute = %s\n" "$put_o" 
					printf "n. di PUT_OP fallite = %s\n" "$put_f" 
				fi

				if [ $uu -eq 1 ]; then  
					printf "n. di UPDATE_OP ricevute = %s\n" "$up_o" 
					printf "n. di UPDATE_OP fallite = %s\n" "$up_f" 
				fi

				if [ $gg -eq  1 ]; then  
					printf "n. di GET_OP ricevute = %s\n" "$get_o" 
					printf "n. di GET_OP fallite = %s\n" "$get_f" 
				fi

				if [ $rr -eq  1 ]; then  
					printf "n. di REMOVE_OP ricevute = %s\n" "$rem_o" 
					printf "n. di REMOVE_OP fallite = %s\n" "$rem_f" 
				fi

				if [ $ccc -eq  1 ]; then  
					printf "n. massimo di connessioni concorrenti raggiunte = %s\n" "$nc" 
				fi

				siz_KB=$((siz * 1000))
				if [ $ss -eq  1 ]; then  
					printf "size attuale della repository in KB = %s\n" "$siz_KB"
				fi

				if [ $oo -eq  1 ]; then  
					printf "n. corrente di oggetti memorizzati = %s\n" "$n_ogg" 
				fi

				if [ "$max_a_nc" -lt "$nc" ] ; then
					max_a_nc=$nc
				fi

				if [ "$max_a_s" -lt "$max_s" ] ; then
					max_a_s=$max_s
				fi

				if [ "$max_a_ng" -lt "$max_ng" ] ; then
					max_a_ng=$max_ng
				fi

			done < $1
		fi

				if [ $perr -eq 1 ]; then  
						s=$(tail -n -1 $1)																					

						#elimino le parti non necessarie
						s=${s#*-}
						i=1
						for n in $s ; do 																					
							if [ $i -eq 1 ]; then   npt=$n ;fi
							if [ $i -eq 2 ] ; then  npf=$n ;fi
							if [ $i -eq 3 ] ; then  nut=$n ;fi
							if [ $i -eq 4 ] ; then  nuf=$n ;fi
							if [ $i -eq 5 ] ; then  ngt=$n ;fi
							if [ $i -eq 6 ] ; then  ngf=$n ;fi
							if [ $i -eq 7 ] ; then  nrt=$n ;fi
							if [ $i -eq 8 ] ; then  nrf=$n ;fi
							if [ $i -eq 9 ] ; then  nkt=$n ;fi
							if [ $i -eq 10 ] ; then  nkf=$n;fi
							let i=i+1
						done

						
						np=$(($npt-$npf))
						np=$(($np * 100))
						np=$(($np/$npt))
					printf "percentuale di put andate a buon fine = %s \n" "$np"

						nu=$(($nut-$nuf))
						nu=$(($nu * 100))
						nu=$(($nu/$nut))
					printf "percentuale di update andate a buon fine = %s \n" "$nu"

						ng=$(($ngt-$ngf))
						ng=$(($ng * 100))
						ng=$(($ng/$ngt))
					printf "percentuale di get andate a buon fine = %s \n" "$ng"

						nr=$(($nrt-$nrf))
						nr=$(($nr * 100))
						nr=$(($nr/$nrt))
					printf "percentuale di remove andate a buon fine = %s \n" "$nr"

						nk=$(($nkt-$nkf))
						nk=$(($nk * 100))
						nk=$(($nk/$nkt))

					printf "percentuale di lock andate a buon fine = %s \n" "$nk"

				fi			

				if [ $mm -eq  1 ]; then  
					printf "n. massimo di connessioni concorrenti raggiunte = %s\n" "$max_a_nc"

					printf "n. massimo di oggetti memorizzati raggiunto = %s\n" "$max_a_ng"

					siz_KB=$((max_a_s * 1000))
					printf "size massima in KB raggiunta = %s\n" "$siz_KB" 
				fi

				if [ $hh -eq  1 ]; then  
					echo per inserire un opzione scriverla separata da uno spazio dopo il nome del file di configurazione
					echo le opzione possiblili sono:
					echo Senza nessuna opzione lo script stampa tutte le statistiche dell’ultimo timestamp.
					echo L’opzione -p stampa il numero di PUT OP ed il numero di PUT OP fallite per tutti i timestamp. 
					echo L’opzione -u stampa il numero di UPDATE OP ed il numero di UP-DATE OP fallite per tutti i timestamp. 
					echo L’opzione -g stampa il numero di GET OP ed il numero di GET OP fallite per tutti i timestamp. 
					echo L’opzione -r stampa il numero di REMOVE OP ed il numero di REMOVE OP fallite per tutti i timestamp. 
					echo L’opzione -c stampa il numero di connessioni per tutti i timestamp. 
					echo L’opzione -s stampa la size in KB per tutti i timestamp. 
					echo L’opzione -o stampa il numero di oggetti nel repository per tutti i timestamp. 
					echo L’opzione -m stampa il massimo numero di connessioni raggiunte dal server, il massimo numero di oggetti memorizzati ed la massima size in KBraggiunta.
					echo L’opzione -per stampa le percentuali delle put, update, get, remove, lock andatr a buon fine
				fi

	fi
else

	#taglio dal file l'ultima riga 
	s=$(tail -n -1 $1)																					

	#elimino le parti non necessarie
	s=${s#*-}																							

	i=1

	#stampo le statistiche dell'ultima riga
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
		if [ $i -eq 11 ] ; then  printf "n. massimo di connessioni concorrenti raggiunte = %s\n" "$n" ;fi   #  il numero massimo o il numero attuale?
		if [ $i -eq 12 ] ; then  printf "size in bytes corrente = %s\n" "$n" ;fi
		if [ $i -eq 13 ] ; then  printf "size massima in bytes raggiunta = %s\n" "$n" ;fi
		if [ $i -eq 14 ] ; then  printf "n. corrente di oggetti memorizzati = %s\n" "$n" ;fi
		if [ $i -eq 15 ] ; then  printf "n. massimo di oggetti memorizzati raggiunto = %s\n" "$n" ;fi

		let i=i+1
	done
fi