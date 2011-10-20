export GINI_HOME=$HOME/gini
export PATH=$PATH:$GINI_HOME/bin
GINI_SRC=$GINI_HOME/gini-src/backend/
cp udp.h $GINI_SRC/include/
cp udp.c $GINI_SRC/src/grouter/
cp message.c $GINI_SRC/src/grouter/
cd $GINI_HOME/gini-src/
scons install
