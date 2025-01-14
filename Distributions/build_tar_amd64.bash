#!/usr/bin/env bash
# Alexis Megas.

if [ ! -r biblioteq.pro ]; then
    echo "Please execute $0 from the primary directory."
    exit 1
fi

# Prepare ./biblioteq.

make distclean 2>/dev/null
mkdir -p ./biblioteq/Documentation
mkdir -p ./biblioteq/SQL
qmake -o Makefile biblioteq.pro && make -j $(nproc)
cp -p ./BiblioteQ ./biblioteq/.
cp -p ./Icons/book.png ./biblioteq/.
cp -p ./SQL/* ./biblioteq/SQL/.
cp -p ./biblioteq.conf ./biblioteq/.
cp -p ./biblioteq.sh ./biblioteq/.
cp -pr ./Data ./biblioteq/.
cp -pr ./Documentation/* ./biblioteq/Documentation/.

# Prepare a tar bundle.

cp $(ldd ./BiblioteQ | awk '{print $3}' | grep -e '^/') ./biblioteq/.
tar -cv -f BiblioteQ-2022.12.25.tar ./biblioteq
make distclean
rm -fr ./biblioteq
