https://github.com/openssl/openssl/blob/master/INSTALL.md
https://github.com/openssl/openssl/blob/master/NOTES-WINDOWS.md
https://www.msys2.org/

PATH=/c/dev/vendor/mingw64/bin:$PATH
pacman -S make
./Configure mingw64 no-comp no-ssl no-ssl2 no-ssl3 enable-fips --prefix=/c/dev/vendor/openssl-3.5.0 --openssldir=/c/dev/vendor/openssl-3.5.0
make
make install
PATH=/c/dev/vendor/openssl-3.5.0/bin:$PATH

openssl fipsinstall -out /c/dev/0_SSL/fipsmodule.cnf -module /c/dev/vendor/openssl-3.5.0/lib64/ossl-modules/fips.so

---

./Configure mingw64 no-comp no-ssl no-ssl2 no-ssl3 enable-fips --prefix=/c/dev/vendor/openssl-3.1.2 --openssldir=/c/dev/vendor/openssl-3.1.2

$ cp ../openssl-3.1.2/providers/fips.so providers/.
$ cp ../openssl-3.1.2/providers/fipsmodule.cnf providers/.
// Note that for OpenSSL 3.1.2 that the `fipsmodule.cnf` file should not
// be copied across multiple machines if it contains an entry for
// `install-status`. (Otherwise the self tests would be skipped).

// Validate the output of the following to make sure we are using the
// OpenSSL 3.1.2 FIPS provider
$ ./util/wrap.pl -fips apps/openssl list -provider-path providers -provider fips -providers

// Now run the current tests using the OpenSSL 3.1.2 FIPS provider.
$ make tests

openssl fipsinstall -module /c/dev/vendor/openssl-3.5.0/lib64/ossl-modules/fips.dll -in /c/dev/vendor/openssl-3.5.0/fipsmodule.cnf  -provider_name fips -verify