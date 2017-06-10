#!/bin/bash
# Print the tree directory structure.
# To install 'tree', use the following:
# sudo apt install tree

NAME=$0
FULL_NAME=$(readlink -f $NAME)
DIR_NAME=$(dirname ${FULL_NAME})
TOP_DIR=${DIR_NAME}/..

tree -dn -I "include|obj|screenShots" -- ${TOP_DIR} | \
# replace top level dir name
sed "s/^\/home.*$/OS/" | \
# replace nested src dirs
sed "/^.* .* .*src$/d" | \
# delete "# of directories"
sed "/directories/d"
