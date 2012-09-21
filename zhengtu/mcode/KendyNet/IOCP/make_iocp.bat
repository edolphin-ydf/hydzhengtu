gcc -O2 -c Acceptor.c Connector.c KendyNet.c link_list.c buffer.c Connection.c rpacket.c wpacket.c
ar -rc iocp.a Acceptor.o Connector.o KendyNet.o link_list.o buffer.o Connection.o rpacket.o wpacket.o