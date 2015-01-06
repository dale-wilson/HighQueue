# Command file to set HSQueue environment
# HSQueue depends on MPC V 3.6 or later. (http://www.ociweb.com/products/mpc)
# HSQueue depends on BOOST V 1.36.0 or later. (http://www.boost.org/)
# HSQueue depends on Xerces V3.0 or later. (http://xerces.apache.org/xerces-c/)
# Customize this file by setting variables to suit your environment
SOURCE="${BASH_SOURCE[0]}"
SOURCE_DIR=`dirname $SOURCE`
export HSQueue_ROOT=`readlink -f $SOURCE_DIR`

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

export PATH=$HSQueue_ROOT/bin:$MPC_ROOT:$PATH
export LD_LIBRARY_PATH=$HSQueue_ROOT/lib:$BOOST_ROOT_LIB:$LD_LIBRARY_PATH

