AC_INIT([0.0], [chxnet examples])
AM_INIT_AUTOMAKE([-Werror -Wall foreign])

: ${CXXFLAGS="-O3"}
AC_PROG_CXX
AC_SUBST(CXXFLAGS, ["$CXXFLAGS -std=c++17"])
AC_CONFIG_FILES(
    Makefile
    example/Makefile
)
AC_OUTPUT
