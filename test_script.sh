#! bin/bash
# test_script.sh
# Script looks inside all sections and find all projects (directory has main.cpp)
# Then the script builds all projects and store the build log into _logs_ directory under the name of the project  
# Sript count all build fails and report the number to slack

# Script look into a folder and test if there is file main.cpp or not 
# 
#



############## SECTIONS TO BUILD ################ 
SECTIONS=("examples" "bootloaders" "customers" "projects" "tests" "firmware")

# Bootloader keyword
BOOTLOADER_KEYWORD="bootloader"
FAILED_PROGRAMS=""
FAIL_COUNTER=0
MISSING_JSON_COUNTER=0
MISSING_JSONS=""

#


## TO DO add argument for ${MAKE_DIR} 
function printHeader(){
echo "" >> _logs_/BUILD_FAILED.log 
echo "-----------------------------------------------" >> _logs_/BUILD_FAILED.log 
printf "PROJECT %s BUILD LOG \n" "$1" >> _logs_/BUILD_FAILED.log
echo "-----------------------------------------------" >> _logs_/BUILD_FAILED.log
echo "" >> _logs_/BUILD_FAILED.log 
}


#echo "test CI VARIABLES"
#echo "${CIRCLE_PROJECT_USERNAME}"
#echo "${CI}"
#echo "${CIRCLECI}"
#echo "${CIRCLE_USERNAME}" 


if [ "${CIRCLECI}" == true ] ; then 
	AUTHOR=${CIRCLE_USERNAME}
	echo "${AUTHOR}: Buildig on Circle CI "
else 
	echo "Building localy"
	AUTHOR="LOCAL COMPUTER"
fi


# Text Colors 
RED='\033[1;31m'
GREEN='\033[1;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color



#JSON_KEYS=("name" "description" "version" "date" "category" "targets")

#function parse_json(){	
#	echo "PARSE JSON"
#
#	EXAMPLE_NAMES+=("`printf "%s" "$value" | grep "\"name\":" | sed 's/^.*:\"\([^\"]*\)\".*/\1/'`")       #PARSING MASTER JARA IS RESPONSIBLE FOR THIS GENIUS LINE OF CODE
#	EXAMPLE_TYPES+=(`printf "%s" "$value" | grep "\"cathegory\":" | sed 's/^.*:\"\([^\"]*\)\".*/\1/'`)
#	let "NUMEBER_OF_EXAMPLES++"
#
#
#	for (( m = 0; m < $NUMEBER_OF_EXAMPLES; m++))
#	do
#		 echo "${EXAMPLE_NAMES[$m]}"
#	done
#		
#}

# Pass name of tested section as an argument
# Without passed argument sctipt search in root directory
function build_section(){
	
	if [ $# != 0 ] && [ -d "$1" ] ; then
		printf "**************************** ${GREEN}%s${NC} ****************************\n" "Building $1 section"
		cd $1
		MAKE_DIR="../" 
		CURRENT_SECTION="${1}/" 
		
	elif [ $# != 0 ] ; then 
		echo "*******************************************************************************************" 
		printf "${RED}Section %s does not exist${NC} \n" "$1"
 
		return 

	else 
		printf "**************************** ${GREEN}%s${NC} ****************************\n" "Building ROOT DIRECTORY"
		MAKE_DIR=""
		CURRENT_SECTION=""
	fi
	
	

	# Get list of all directories
	dirs=(`ls -d */`)


 	#********************************************************************************************************************************************************************************
	# Go through all directories found in the section 
	for i in ${dirs[@]}; do

		# DIR name without a slash at the end
		DIR_NAME=${i:0:${#i}-1}
		TMP_MISSING=false
		# Reset missing json variable


		# If is searching in ROOT directory, dont look into section
		if [ $# == 0 ]; then
			JUMP=false
			for j in ${SECTIONS[@]}; do

				if [ $DIR_NAME == $j ]; then
					#printf "Found section %s \n" "$j" 			
					JUMP=true
					break
				fi 

			done

			# Continue to next directory
			if $JUMP ; then
				continue
			fi

		fi 

		
		# Dont look up in _logs_ and _libs_
		if [ $DIR_NAME == "_logs_" ] || [ $DIR_NAME == "_libs_" ] ; then
			continue
		fi


		# If searching in section, check if the section is called "examples"
		if [ $# != 0 ] ; then 

		# If is example section, look up for json 
			if [ $1 == "examples" ] ; then 

				if ! [ -f $i/readme.json ]; then
					
					let "MISSING_JSON_COUNTER++"
					MISSING_JSONS=$(printf "$MISSING_JSONS \n $DIR_NAME")
					TMP_MISSING=true
				fi
			fi 
		fi

		printf "%s" "Testing project $i...."
		if [ -a $i"main.cpp" ];
			then
			
				printf "%s" "Building"
				
				# Test if the keyword is in the directory name
				BOOTLOADER="FALSE"
				
				if test "${i#*$BOOTLOADER_KEYWORD}" != "$i" 
				then
					BOOTLOADER="TRUE"
				fi
					
				if [ $BOOTLOADER == "TRUE" ]; then
						
						#BOOT AS BOOTLOADER 
						node ${MAKE_DIR}_makescript_.js ${CURRENT_SECTION}${DIR_NAME} BYZANCE_IODAG3E_BOOTLOADER &>${MAKE_DIR}_logs_/$DIR_NAME.log
						if [ $? == 0 ]; then
							printf "\t\t${GREEN}%s${NC}" "BUILD SUCCEEDED (BOOTLOADER)"
						else 
							let "FAIL_COUNTER++"
							printf "\t\t${RED}%s${NC}" "BUILD FAILED (BOOTLOADER)"

							FAILED_PROGRAMS=$(printf "$FAILED_PROGRAMS\n $DIR_NAME")
						fi	
						
						node ${MAKE_DIR}_makescript_.js ${CURRENT_SECTION}${DIR_NAME} BYZANCE_IODAG3E_BOOTLOADER clean &>/dev/null
				else
					
						#BOOT AS MAIN FIRMWARE
						node ${MAKE_DIR}_makescript_.js ${CURRENT_SECTION}${DIR_NAME} BYZANCE_IODAG3E &>${MAKE_DIR}_logs_/$DIR_NAME.log
						
						if [ $? == 0 ]; then
							printf "\t\t${GREEN}%s${NC}" "BUILD SUCCEEDED"
						else 
							let "FAIL_COUNTER++"
							printf "\t\t${RED}%s${NC}" "BUILD FAILED"

							# Add Failed program name to failed programs list 
							FAILED_PROGRAMS=$(printf "$FAILED_PROGRAMS\n $DIR_NAME")
							
							# printHeader $DIR_NAME ${MAKE_DIR} # Function printHeader not used at the moment .. need to add argument for ${MAKE_DIR}
							printf "\n" >> ${MAKE_DIR}_logs_/BUILD_FAILED.log 
							printf '*************************************************** \n' >> ${MAKE_DIR}_logs_/BUILD_FAILED.log 
							printf "PROJECT %s BUILD LOG \n" "$DIR_NAME" >> ${MAKE_DIR}_logs_/BUILD_FAILED.log
							printf '*************************************************** \n' >> ${MAKE_DIR}_logs_/BUILD_FAILED.log
							printf "\n" >> ${MAKE_DIR}_logs_/BUILD_FAILED.log 
							cat ${MAKE_DIR}_logs_/$DIR_NAME.log >> ${MAKE_DIR}_logs_/BUILD_FAILED.log 
			
						fi		
						
						node ${MAKE_DIR}_makescript_.js ${CURRENT_SECTION}${DIR_NAME} BYZANCE_IODAG3E clean &>/dev/null
				fi

				if [ "${TMP_MISSING}" == true ] ; then 
					printf "${RED} (MISSING readme.json)${NC} \n"
				else 
					printf "\n"
				fi

		else
			#printf "%s" "Testing file $i...."
			printf "\t\t${YELLOW}%s${NC} \n" "DOESN'T HAVE main.cpp";
		fi

	done
	#**********************************************************************************************************************************************************************************




	# Return back to root directory
	if [ $# != 0 ]; then
		cd ..
	fi

}



# Inti log file if does not exist
if ! [ -d "_logs_" ]; then
	mkdir _logs_
fi	

echo "ERROR LOG " > ${MAKE_DIR}_logs_/BUILD_FAILED.log 

# Search in HW libs for projects not located in any section
build_section 

# Search in all sections 
for i in ${SECTIONS[@]}; do 
	build_section ${i};
done


#for (( n = 0; n < FAIL_COUNTER; n++))
#do 
#	echo "$n"
#	echo "${FAILED_PROGRAMS[$n]}"
#done

if [ $FAIL_COUNTER \> 0 ] && [ $MISSING_JSON_COUNTER \> 0 ] ; then
	echo "Repository has some fails and also is missing some jsons"
	printf "Send Warning message to slack \n"
	#curl -X POST --data-urlencode "payload={\"channel\": \"#hardware\", \"username\": \"Circle CI\", \"text\": \" Something gets wrong. Last push to HW-LIBS failed. \n Push author: ${AUTHOR} \n Number of fails: $FAIL_COUNTER \n \n Failed programs: ${FAILED_PROGRAMS} \n \n Some examples are also missing readme.json files: ${MISSING_JSONS}.\"}" https://hooks.slack.com/services/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 &>/dev/null

elif [ $FAIL_COUNTER \> 0 ] ; then 
	echo "Repository has some fails"
	printf "Send Warning message to slack \n"
	#curl -X POST --data-urlencode "payload={\"channel\": \"#hardware\", \"username\": \"Circle CI\", \"text\": \" Something gets wrong. Last push to HW-LIBS failed. \n Push author: ${AUTHOR} \n Number of fails: $FAIL_COUNTER \n \n Failed programs: ${FAILED_PROGRAMS}.\"}" https://hooks.slack.com/services/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 &>/dev/null

elif [ $MISSING_JSON_COUNTER \> 0 ] ; then 
	echo "Repository is missing some jsons"
	printf "Send Warning message to slack \n"
	#curl -X POST --data-urlencode "payload={\"channel\": \"#hardware\", \"username\": \"Circle CI\", \"text\": \" Some examples are missing readme.json files \n Push author: ${AUTHOR} \n \n Missing jsons in: ${MISSING_JSONS}.\"}" https://hooks.slack.com/services/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 &>/dev/null

fi


echo "TEST DONE"
printf "Number of failed builds %s \n" "$FAIL_COUNTER"
exit $FAIL_COUNTER

 



