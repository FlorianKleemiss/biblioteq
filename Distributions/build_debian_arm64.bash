#!/usr/bin/env bash
# Alexis Megas.

if [ ! -x /usr/bin/dpkg-deb ]; then
    echo "Please install dpkg-deb."
    exit
fi

if [ ! -x /usr/bin/fakeroot ]; then
    echo "Please install fakeroot."
    exit 1
fi

if [ ! -r biblioteq.arm.pro ]; then
    echo "Please execute $0 from the primary directory."
    exit 1
fi

# Preparing ./usr/local/biblioteq:

make distclean 2>/dev/null
mkdir -p ./usr/local/biblioteq/Documentation
mkdir -p ./usr/local/biblioteq/SQL
qmake -o Makefile biblioteq.arm.pro && make -j $(nproc)
cp -p ./BiblioteQ ./usr/local/biblioteq/.
cp -p ./Icons/book.png ./usr/local/biblioteq/.
cp -p ./SQL/* ./usr/local/biblioteq/SQL/.
cp -p ./biblioteq.conf ./usr/local/biblioteq/.
cp -p ./biblioteq.sh ./usr/local/biblioteq/.
cp -pr ./Data ./usr/local/biblioteq/.
cp -pr ./Documentation/* ./usr/local/biblioteq/Documentation/.

# Preparing BiblioteQ-x.deb:

mkdir -p biblioteq-debian/usr/local
mkdir -p biblioteq-debian/usr/share/applications
cp -p ./biblioteq.desktop biblioteq-debian/usr/share/applications/.
cp -pr ./PIOS64 biblioteq-debian/DEBIAN
cp -r ./usr/local/biblioteq biblioteq-debian/usr/local/.
fakeroot dpkg-deb --build biblioteq-debian BiblioteQ-2022.12.25_arm64.deb
make distclean
rm -fr ./usr
rm -fr biblioteq-debian
