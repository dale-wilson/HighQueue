# Command file to set HighQueue environment
# HighQueue depends on MPC V 4.1 or later. (http://www.ociweb.com/products/mpc)
# HQTest and HQPerformance depend on BOOST Test.  Tested with 1.57.0. (http://www.boost.org/)
# Customize this file by setting variables to suit your environment
SOURCE="${BASH_SOURCE[0]}"
SOURCE_DIR=`dirname $SOURCE`
export HighQueue_ROOT=`readlink -f $SOURCE_DIR`

if test "$MPC_ROOT" = ""
then
  export MPC_ROOT=~/MPC
fi

if test "$BOOST_ROOT" = ""
then
  export BOOST_ROOT=/usr/local
fi

if test "$BOOST_ROOT_LIB" = ""
then
  export BOOST_ROOT_LIB=$BOOST_ROOT/lib
fi

if test "$BOOST_VERSION" = ""
then
  export BOOST_VERSION=boost-1_57
fi

export PATH=$HighQueue_ROOT/bin:$MPC_ROOT:$PATH
export LD_LIBRARY_PATH=$HighQueue_ROOT/lib:$BOOST_ROOT_LIB:$LD_LIBRARY_PATH

