#!/bin/bash

while read; do
	filename=`echo $REPLY | cut '-d,' -f 1`
	edition=`echo $REPLY | cut '-d,' -f 2`
	editionLong=`echo $REPLY | cut '-d,' -f 3`

	wget http://www.crystalkeep.com/magic/rules/oracle/oracle-$filename.txt
	mv oracle-$filename.txt "${edition}_${editionLong}.txt"
done
