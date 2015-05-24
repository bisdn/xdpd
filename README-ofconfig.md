OFconfig support for xDPd
=========================

There is an initial implementation for OFConfig 1.1.1 for xDPd. Please refer to for more details:

	https://github.com/bisdn/of-config

xDPd XMP plugin required
========================

Please refer to the manual in the ofconfig repository. Remember to compile with xmp support:

	sh# ../configure --with-plugins="xmp" 
	sh# make  
	sh# make install

