#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([luaproc], [1.0.0], [smherwig@gmail.com], [luaproc],
    [http://smherwig.org])
AM_INIT_AUTOMAKE
LT_PREREQ([2.2])
LT_INIT

AC_CONFIG_SRCDIR([include/lua-proc.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for header files.
missing_header=no
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h lua5.1/lua.h lua5.1/lualib.h lua5.1/lauxlib.h], 
    [], 
    [missing_header=yes])

if test "x${missing_header}" = xyes; then
    AC_MSG_ERROR([Missing required header file])
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T

AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset strdup strerror strtoul])

AC_CONFIG_FILES([Makefile
            include/Makefile
            share/Makefile
            src/Makefile])

AC_OUTPUT

echo \
"-----------------------------------------------------------

${PACKAGE_NAME} version ${PACKAGE_VERSION}

Prefix: '${prefix}'

Now type 'make @<:@<taret>@:>@'
    where the optional <target> is:
        all         - build all binaries
        install     - install everything

-----------------------------------------------------------"

