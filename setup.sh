# Command file to set HighQueue environment
# Depemds om MPC https://github.com/DOCGroup/MPC
# HQTest and HQPerformance depend on BOOST Test 1.57.0 or later.  (http://www.boost.org/)
# Customize this file by setting variables to suit your environment
SOURCE="${BASH_SOURCE[0]}"
SOURCE_DIR=`dirname $SOURCE`
export HighQueue_ROOT=`readlink -f $SOURCE_DIR`

if test "$MPC_ROOT" = ""
then
  export MPC_ROOT=/MPC
fi

if test "$BOOST_ROOT" = ""
then
  export BOOST_ROOT=/usr/local
fi

if test "$BOOST_LIB_PATH" = ""
then
  export BOOST_LIB_PSTH=$BOOST_ROOT/lib
fi

if test "$BOOST_VERSION" = ""
then
  export BOOST_VERSION=boost-1_61
fi

if test "$KERNEL_INCLUDE" = ""
then
  export KERNEL_INCLUDE=/usr/src/kernels/3.10.0-327.13.1.el7.xppsl_1.3.3.151.x86_64
  export LINUX_INCLUDE=$KERNEL_INCLUDE/include
  export ARCH_INCLUDE=$KERNEL_INCLUDE/arch/x86/include
fi

export PATH=$HighQueue_ROOT/bin:$MPC_ROOT:$PATH
export LD_LIBRARY_PATH=$HighQueue_ROOT/lib:$BOOST_LIB_PATB:$LD_LIBRARY_PATH

