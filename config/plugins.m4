##Management plugins system is in charge of compiling
##management plugins defined at configure time

#Just for auto-reconf to build Makefile.am
#m4_include([config/plugins_list.m4])


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

#Check if plugins do exist
for PLUGIN in $PLUGINS; do
	
	#Define plugin directory
	DIRECTORY="$srcdir/src/xdpd/management/plugins/$PLUGIN"

	#Check directory
	if ! test -d "$DIRECTORY" ; then
		AC_MSG_ERROR([Plugin [$PLUGIN]: source folder '$DIRECTORY' not found!])
	fi

	#Check M4 
	#if ! test -f "$DIRECTORY/Makefile.am" ; then
	#	AC_MSG_ERROR([Plugin [$PLUGIN]: $DIRECTORY/Makefile.am not found!])
	#fi


	#Check Makefile
	#if ! test -f "$DIRECTORY/Makefile.am" ; then
	#	AC_MSG_ERROR([Plugin [$PLUGIN]: $DIRECTORY/Makefile.am not found!])
	#fi


	#AC_CONFIG_FILES([
	#	src/xdpd/management/plugins/$PLUGIN/Makefile
	#])

	#Add plugin to the list of PLUGIN sources
	PLUGIN_LIBS="plugins/$PLUGIN/libxdpd_mgmt_$PLUGIN.la"
	PLUGIN_DIRS="plugins/$PLUGIN"
	
	#Set flag
	eval with_$PLUGIN="yes"
done

#Define subst
AC_SUBST(PLUGIN_LIBS)
AC_SUBST(PLUGIN_DIRS)
AC_DEFINE(PLUGINS, $PLUGINS)
AC_MSG_RESULT($PLUGINS)


#
# M4 
#

#QMF
if ! test -z "$with_qmf" ; then
	m4_include([config/plugin_qmf.m4])
fi

#CLI
if ! test -z "$with_cli" ; then
	m4_include([config/plugin_cli.m4])
fi

