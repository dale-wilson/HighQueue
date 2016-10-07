##! /etc/bash
# Copyright  Object Computing, Inc.
# All rights reserved.
# See the file license.txt for licensing information.
#
# This batch file runs MWC which is part of the MPC package to create
# make files to build HighQueue
#
# Note: -expand_vars -use_env options force MPC to expand $(BOOST_ROOT) into the absolute path.  This avoids
#        a problem that happened when people were starting Visual Studio from the Start menu rather than
#        from the command line where the BOOST_ROOT environment had been defined.

$MPC_ROOT/mwc.pl -type make  -use_env HighQueue.mwc
