##Management plugins system is in charge of compiling
##management plugins defined at configure time

AC_MSG_CHECKING(the plugins to be compiled...)

AC_ARG_WITH(plugins,
        	AS_HELP_STRING([--with-plugins="plugin1 ... pluginN"], [Compile with the plugins listed in the argument list [default plugins="cli"]]),
 [ 
	#They provide a list of plugins
	PLUGINS=$withval
 ],[
	#Use default
	PLUGINS="cli"
	
])

AC_DEFINE(PLUGINS, $PLUGINS)
AC_MSG_RESULT($PLUGINS)

#Check if plugins do exist
for PLUGIN in $PLUGINS; do
	
	#Define plugin directory
	DIRECTORY="$srcdir/src/xdpd/management/plugins/$PLUGIN"

	#Check directory
	if ! test -d "$DIRECTORY" ; then
		AC_MSG_ERROR([Plugin [$PLUGIN]: source folder '$DIRECTORY' not found!])
	fi

	#Check M4 
	#if ! test -f "$srcdir/config/plugin_$PLUGIN.m4" ; then
	#	AC_MSG_ERROR([Plugin [$PLUGIN]: m4 script not found!])
	#fi

	#Check Makefile
	if ! test -f "$DIRECTORY/Makefile.am" ; then
		AC_MSG_ERROR([Plugin [$PLUGIN]: $DIRECTORY/Makefile.am not found!])
	fi


	AC_CONFIG_FILES([
		src/xdpd/management/plugins/$PLUGIN/Makefile
	])

	#Add plugin to the list of PLUGIN sources
	#for PLUGIN_SRC in $PLUGIN_SRCS; do
	#	__PLUGIN_SRCS="$__PLUGIN_SRCS plugins/$PLUGIN/$PLUGIN_SRC"
	#done
	#__PLUGIN_HEADERS="$__PLUGIN_HEADERS $PLUGIN_HEADER"
	#__PLUGIN_CLASSES="$__PLUGIN_CLASSES $PLUGIN_CLASS"
	PLUGIN_LIBS="plugins/$PLUGIN/libxdpd_mgmt_$PLUGIN.la"
	PLUGIN_DIRS="plugins/$PLUGIN"
	
	#Reset
	#PLUGIN_HEADER=""
	#PLUGIN_SRCS=""
	#PLUGIN_CLASS=""
done

AC_SUBST(PLUGIN_LIBS)
AC_SUBST(PLUGIN_DIRS)
#AC_SUBST(PLUGIN_SRCS, $__PLUGIN_SRCS)
#AC_SUBST(PLUGIN_HEADERS, $__PLUGIN_HEADERS)
#AC_SUBST(PLUGIN_CLASSES, $__PLUGIN_CLASSES)
#AC_MSG_RESULT("PM headers: $__PLUGIN_HEADERS srcs: $PLUGIN_SRCS classes: $PLUGIN_CLASSES")

