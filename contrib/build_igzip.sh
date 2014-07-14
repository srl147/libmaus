#! /bin/bash
function download
{
	CURL=`which curl`
	WGET=`which wget`
	
	if [ -e ${CURL} ] ; then
		curl ${1} > `basename ${1}`
	elif [ -e ${WGET} ] ; then
		wget ${1}
	else
		echo "No download program found"
		exit 1
	fi
}

if [ ! -f igzip-src.tar.gz ] ; then
	if [ ! -f igzip_042.zip ] ; then
		download https://software.intel.com/sites/default/files/managed/2d/63/igzip_042.zip
	fi

	mkdir -p igzip-src
	cd igzip-src
	unzip ../igzip_042.zip
	cd ..

	tar czf igzip-src.tar.gz igzip-src
	rm -fR igzip-src
fi

YASMVERSION=1.2.0

if [ ! -f yasm-${YASMVERSION}.tar.gz ] ; then
	download http://www.tortall.net/projects/yasm/releases/yasm-${YASMVERSION}.tar.gz
fi

BASEDIR=${PWD}

tar xzf yasm-${YASMVERSION}.tar.gz
mv yasm-${YASMVERSION} yasm-${YASMVERSION}-src
mkdir -p yasm-${YASMVERSION}-build
cd yasm-${YASMVERSION}-build
../yasm-${YASMVERSION}-src/configure --prefix=${BASEDIR}/yasm-bin/${YASMVERSION}
make
make install
cd ..
rm -fR yasm-${YASMVERSION}-src yasm-${YASMVERSION}-build

tar xzf igzip-src.tar.gz
cd igzip-src/igzip
sed < Makefile "s|^YASM *:= *.*|YASM := ${BASEDIR}/yasm-bin/${YASMVERSION}/bin/yasm|" >Makefile.patched
mv Makefile.patched Makefile
sed < options.inc "s|^;%define GENOME_SAM|%define GENOME_SAM|;s|^;%define GENOME_BAM|%define GENOME_BAM|;s|;%define ONLY_DEFLATE|%define ONLY_DEFLATE|" > options.inc.patched
mv options.inc.patched options.inc
if [ "`uname`" = "Darwin" ] ; then
	sed < Makefile "s|-f elf64|-f macho64|;s|-g dwarf2|-g null --prefix=_|;s|-shared|-dynamiclib -current_version 1.0 -compatibility_version 1.0 -fvisibility=hidden -Wl,-install_name,\$@|;s|libigzip0.so|libigzip0.dylib|;s|libigzip0c.so|libigzip0c.dylib|;s|libigzip1.so|libigzip1.dylib|;s|libigzip1c.so|libigzip1c.dylib|" > Makefile.patched
	mv Makefile.patched Makefile
fi
make
cd ../..

SOSUFFIX=.so
if [ "`uname`" = "Darwin" ] ; then
	SOSUFFIX=.dylib
fi

mkdir -p igzip-bin
mkdir -p igzip-bin/lib
mkdir -p igzip-bin/include
for i in igzip-src/igzip/*${SOSUFFIX} igzip-src/igzip/*.a ; do
	cp -p $i igzip-bin/lib/
done
for i in igzip-src/include/types.h igzip-src/igzip/c_code/igzip_lib.h igzip-src/igzip/c_code/internal_state_size.h ; do
	cp -p $i igzip-bin/include/
done

cd igzip-bin/include
sed <igzip_lib.h "s|types.h|igzip_lib_types.h|" >igzip_lib.h.patched
mv igzip_lib.h.patched igzip_lib.h
mv types.h igzip_lib_types.h
cd ../..

tar czf igzip-bin.tar.gz igzip-bin

rm -fR igzip-src
rm -fR igzip-bin
rm -fR ${BASEDIR}/yasm-bin

rm -fR yasm-${YASMVERSION}.tar.gz
rm -fR igzip-src.tar.gz

if [ $# -ge 1 ] ; then
	tar xzvf igzip-bin.tar.gz
	cd igzip-bin
	INSTDIR=$1
	mkdir -p ${INSTDIR}
	tar cf - . | tar xf - -C ${INSTDIR}
	
	if [ "`uname`" = "Darwin" ] ; then
		SDIR=${PWD}
		cd ${INSTDIR}/lib
		for i in *.dylib ; do install_name_tool -change $i ${PWD}/$i -id ${PWD}/${i} ${i} ; done
		cd ${SDIR}
	fi
	
	cd ..
	rm -fR igzip-bin
fi
