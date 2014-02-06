##Management plugins system is in charge of compiling
##management plugins defined at configure time

#Define helpful ADD_PLUGIN MACRO
AC_DEFUN([ADD_PLUGIN], [AM_CONDITIONAL(WITH_MGMT_[]m4_translit($1, `a-z', `A-Z'), test "$with_mgmt_$1" = yes)
AM_COND_IF(WITH_MGMT_[]m4_translit($1, `a-z', `A-Z'), [m4][_include([config/plugin_$1.m4])])
])

#Messages
AC_MSG_CHECKING(the plugins to be compiled...)

AC_ARG_WITH(plugins,
        	AS_HELP_STRING([--with-plugins="plugin1 ... pluginN"], [Compile with the plugins listed in the argument list [default plugins="config"]]),
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
		AC_MSG_ERROR([Plugin {$PLUGIN} source folder '$DIRECTORY' not found!])
	fi

	#Add plugin to the list of PLUGIN sources
        PLUGIN_LIBS+="$PLUGIN/libxdpd_mgmt_$PLUGIN.la "
        PLUGIN_DIRS+="$PLUGIN "
	
	#Set flag
	eval with_mgmt_$PLUGIN="yes"
done

#Define subst
AC_SUBST(PLUGIN_LIBS)
AC_SUBST(PLUGIN_DIRS)
AC_MSG_RESULT($PLUGINS)

#Force recompiling of plugin_manager
touch $srcdir/src/xdpd/management/plugins/pm_timestamp.h

#
# AVAILABLE PLUGINS
#
# ADD_PLUGIN() MACRO does an m4_include conditionally, depending on the with-plugins value
# under the hood.
#
# Warning: plugin names MUST be in lowercase

#CONFIG
ADD_PLUGIN(config)

#QMF
ADD_PLUGIN(qmf)

#XMP
ADD_PLUGIN(xmp)

#REST
ADD_PLUGIN(rest)

#Example
ADD_PLUGIN(example)

#Add more here [+]...
