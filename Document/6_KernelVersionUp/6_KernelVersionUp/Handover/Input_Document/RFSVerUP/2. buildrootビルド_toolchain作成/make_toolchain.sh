#!/bin/bash
CWD=`pwd`
INPUT=${CWD}/buildroot/output/host
OUTPUT=${CWD}/toolchain
#COMMAND="addr2line ar as c++ c++filt cpp elfedit g++ gcc gcc-5.2.1 gcc-ar gcc-nm gcc-ranlib gcov gcov-tool gdb gfortran gprof ld ld.bfd nm objcopy objdump ranlib readelf size strings strip"
COMMAND="addr2line ar as c++ c++filt cpp elfedit g++ gcc gcc-ar gcc-nm gcc-ranlib gcov gcov-tool gdb gprof ld ld.bfd nm objcopy objdump ranlib readelf size strings strip"

# init
rm -rf ${OUTPUT}
cp -a ${INPUT} ${OUTPUT}
rm -f ${OUTPUT}/*.txt
rm -rf ${OUTPUT}/aarch64-buildroot-linux-gnu/lib64/*

# copy files from RFS
cd ${OUTPUT}/aarch64-buildroot-linux-gnu
mv libc libc_bak
mkdir libc
cp -a ${CWD}/buildroot/output/host/usr/aarch64-buildroot-linux-gnu/sysroot/* libc
cd ${CWD}

# setup s800-linux
mkdir -p ${OUTPUT}/s800-linux/bin
cd ${OUTPUT}/s800-linux/bin
for TARGET in ${COMMAND}
do
	ln -s ../../bin/aarch64-buildroot-linux-gnu-${TARGET} s800-linux-${TARGET}
done

cd ${OUTPUT}/bin
ls aarch64-buildroot-linux-gnu-*.br_real -1 | sed -r "s/aarch64-buildroot-linux-gnu-(.*)\.br_real/ln -s aarch64-buildroot-linux-gnu-\1.br_real s800-linux-\1.br_real/g" | bash
ls aarch64-buildroot-linux-gnu-* -1 | sed -r "s/aarch64-buildroot-linux-gnu-(.*)/ln -s aarch64-buildroot-linux-gnu-\1 aarch64-linux-gnu-\1/g" | bash

# ファイル名のパターンに一致するファイルをリストアップし、シンボリックリンクを作成する
#for file in aarch64-buildroot-linux-gnu-*.br_real; do
#	if [[ $file =~ aarch64-buildroot-linux-gnu-(.*)\.br_real ]]; then
#        link_name="s800-linux-${BASH_REMATCH[1]}.br_real"
#        ln -s "$file" "$link_name"
#    fi
#done
#
#for file in aarch64-buildroot-linux-gnu-*; do
#	if [[ $file =~ aarch64-buildroot-linux-gnu-(.*) ]]; then
#        link_name="aarch64-linux-gnu-${BASH_REMATCH[1]}.br_real"
#        ln -s "$file" "$link_name"
#    fi
#done

#旧buildrootを互換のためにシンボリックリンク作成
cd ${OUTPUT}
ln -s aarch64-buildroot-linux-gnu aarch64-linux-gnu 
cd ${OUTPUT}/aarch64-linux-gnu/include/c++
ln -s 11.4.0 5.2.1
cd ${OUTPUT}/aarch64-linux-gnu/include/c++/5.2.1
ln -s aarch64-buildroot-linux-gnu aarch64-linux-gnu
cd ${OUTPUT}/lib/gcc
ln -s aarch64-buildroot-linux-gnu aarch64-linux-gnu
cd ${OUTPUT}/lib/gcc/aarch64-linux-gnu
ln -s 11.4.0 5.2.1
cd ${OUTPUT}/lib/gcc/aarch64-linux-gnu
cd ${OUTPUT}/

cd ${CWD}

# change path
for TARGET in `find toolchain/aarch64-linux-gnu/libc/usr/lib/pkgconfig -name "*.pc"`
do
	sed -e 's/^prefix=\/usr/prefix=\/opt\/marvell\/s800\/toolchain\/aarch64-linux-gnu\/libc\/usr/' ${TARGET} > tmp1.txt
	sed -e 's/^exec_prefix=\/usr/exec_prefix=\/opt\/marvell\/s800\/toolchain\/aarch64-linux-gnu\/libc\/usr/' tmp1.txt > tmp2.txt
	sed -e 's/^libdir=\/usr\/lib/libdir=\/opt\/marvell\/s800\/toolchain\/aarch64-linux-gnu\/libc\/usr\/lib/' tmp2.txt > tmp3.txt
	sed -e 's/^libdir=\/lib64/libdir=\/opt\/marvell\/s800\/toolchain\/aarch64-linux-gnu\/libc\/usr\/lib64/' tmp3.txt > tmp4.txt
	sed -e 's/^includedir=\/usr\/include/includedir=\/opt\/marvell\/s800\/toolchain\/aarch64-linux-gnu\/libc\/usr\/include/' tmp4.txt > ${TARGET}
	rm tmp1.txt tmp2.txt tmp3.txt tmp4.txt 
done

chmod -R 755 ${OUTPUT}
find ${OUTPUT} -name "*.la" | xargs rm 
tar -czf ${OUTPUT}.tar.gz ${OUTPUT}

exit 0
