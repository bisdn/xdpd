#Define ALWAYS WITH_MGMT_$PLUGIN
AC_DEFINE(WITH_MGMT_YAML_CONFIG)

#LIB checks
AC_LANG_PUSH([C++])

PKG_CHECK_MODULES([YAML_CPP], [yaml-cpp], [have_yaml_cpp=yes], AC_MSG_ERROR([libyaml-cpp not found.]))
if (test "${have_yaml_cpp}" = "yes"); then
        CXXFLAGS+=" $YAML_CPP_CFLAGS"
        LIBS+=" $YAML_CPP_LIBS"
fi

AC_LANG_POP([C++])

#Add files
AC_CONFIG_FILES([
	src/xdpd/management/plugins/yaml_config/Makefile
	src/xdpd/management/plugins/yaml_config/interfaces/Makefile
	src/xdpd/management/plugins/yaml_config/openflow/Makefile
	src/xdpd/management/plugins/yaml_config/system/Makefile
])


