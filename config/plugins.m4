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
	PLUGINS="config"
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
        PLUGIN_LIBS+="plugins/$PLUGIN/libxdpd_mgmt_$PLUGIN.la "
        PLUGIN_DIRS+="plugins/$PLUGIN "
	
	#Set flag
	eval with_mgmt_$PLUGIN="yes"
done

#Define subst
AC_SUBST(PLUGIN_LIBS)
AC_SUBST(PLUGIN_DIRS)
#AC_DEFINE(PLUGINS, $PLUGINS)
#AC_SUBST(PLUGINS, $PLUGINS)
AC_MSG_RESULT($PLUGINS)

#Force recompiling of plugin_manager
touch $srcdir/src/xdpd/management/pm_timestamp.h

#
# Include M4 plugin scripts conditionally
#

#CONFIG
AM_CONDITIONAL(WITH_MGMT_CONFIG, test "$with_mgmt_config" = yes)
AM_COND_IF(WITH_MGMT_CONFIG, [m4_include([config/plugin_config.m4])],[])

#QMF
AM_CONDITIONAL(WITH_MGMT_QMF, test "$with_mgmt_qmf" = yes)
AM_COND_IF(WITH_MGMT_QMF, [m4_include([config/plugin_qmf.m4])],[])

#Add more here..

#Example
AM_CONDITIONAL(WITH_MGMT_EXAMPLE, test "$with_mgmt_example" = yes)
AM_COND_IF(WITH_MGMT_EXAMPLE, [m4_include([config/plugin_example.m4])],[])

