#!/bin/bash
# This script generates coloseum banner (prestavenie.atl)
# from files:
# predstavenie.head - title of the banner
# predstavenie.tail - footer of the banner
# program.atl - playlist of scripts for talker

src_head="predstavenie.head"
src_tail="predstavenie.tail"
src_play="program.atl"
dir_colos="amfiteater"
dir_scripts="scripts"
dir_banners="banners"
ext_script=".atl"
ext_banner=".pokec"

output="predstavenie.atl"
banners=""

echo -n "Inserting HEAD..."

cat $src_head > $output
if [ $? -eq 0 ]
then
	echo "done!"
else
	echo "error!"
	exit 1
fi

echo "Processing '$src_play':"

declare -i l=0
declare -i i=0
old_ifs=$IFS
IFS=""
while read line
do
	l+=1
	echo -n "Processing line #$l..."
	cas=$(echo $line | awk '{ print $1; }')
	script_file=$(echo $line | awk '{ print $2; }')
	nazov=$(echo $line | cut -c 42-)
	if [ -n "$cas" ] && [ -n "$script_file" ] && [ -n "$nazov" ]
	then
		i+=1
		echo "   ~HT$cas   ~HG$nazov" >> $output
		if [ $? -eq 0 ]
		then
			banner_file=${script_file//$dir_colos\/$dir_scripts/$dir_banners}
			banner_file=${banner_file//$ext_script/$ext_banner}
			banners[$i]="$banner_file"
			shows[$i]="$nazov"
			echo "done!"
		else
			echo "error!"
			echo "Could not write to '$output'!"
			exit 1
		fi
	else
		echo "warning!"
		echo "Skipped line #$i: $line"
	fi
done < $src_play

IFS=$old_ifs
echo "Inserting show infos:"
echo >> $output

for id in $(seq 1 $i)
do
	echo -n "Looking for info about show '${shows[$id]}'..."
	if [ -f ${banners[$id]} ]
	then
		cat ${banners[$id]} >> $output
		if [ $? -eq 0 ]
		then
			echo "inserted!"
		else
			echo "failed writing!"
			exit 1
		fi
		echo >> $output
		if [ ! $? -eq 0 ]
		then
			echo "Warning: could not insert empty line!"
		fi
	else
		echo "not found!"
	fi
done
echo -n "Inserting TAIL..."
cat $src_tail >> $output
if [ $? -eq 0 ]
then
	echo "done!"
else
	echo "error!"
	exit 1
fi
