#!/bin/bash
pushd $1 > /dev/null
for pkg in *; do
	INCLUDE_DIR="$pkg/include"
	if [ -d "$INCLUDE_DIR" ]; then
		if [[ "$(ls -1 $INCLUDE_DIR | wc -l)" == "1" ]] && [ -d "$INCLUDE_DIR/$pkg" ] && [[ "$(ls -1 $INCLUDE_DIR/$pkg | wc -l)" == "1" ]] && [ -d "$INCLUDE_DIR/$pkg/$pkg" ]; then
			echo "$1/$INCLUDE_DIR/$pkg"
		else
			echo "$1/$INCLUDE_DIR"
		fi
	fi
done
popd > /dev/null
