CC:=gcc
.PHONY: single_conn single_conn_clean
.PHONY: single_conn single_conn_clean multi_conn_threads 
single_conn_executables:=single_conn_server single_conn_client

single_conn_server: single_conn_server.c
	-$(CC) -o $@ $^
single_conn_client: single_conn_client.c
	-$(CC) -o $@ $^
single_conn: $(single_conn_executables)
single_conn_clean:
	-rm ./$(word 1, $(single_conn_executables)) ./$(word 2, $(single_conn_executables))
without_buf_test: without_buf_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@
line_buf_test: line_buf_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@
multi_conn_threads_executables:=multi_conn_threads_server multi_conn_threads_client
multi_conn_threads_server: multi_conn_threads_server.c
	-$(CC) -o $@ $^
multi_conn_threads_client: multi_conn_threads_client.c
	-$(CC) -o $@ $^
multi_conn_threads: $(multi_conn_threads_executables)
multi_conn_threads_clean:
	-rm ./$(firstword $(multi_conn_threads_executables)) ./$(lastword $(multi_conn_threads_executables))
.PHONY: single_conn single_conn_clean multi_conn_threads multi_conn_threads_clean multi_conn_processes multi_conn_processes_clean
multi_conn_processes_executables:=multi_conn_processes_server multi_conn_processes_client

multi_conn_processes_server: multi_conn_processes_server.c
	-$(CC) -o $@ $^

multi_conn_processes_client: multi_conn_processes_client.c
	-$(CC) -o $@ $^

multi_conn_processes: $(multi_conn_processes_executables)

multi_conn_processes_clean:
	-rm ./$(word 1, $(multi_conn_processes_executables)) ./$(word 2, $(multi_conn_processes_executables))
.PHONY: single_conn single_conn_clean multi_conn_threads multi_conn_threads_clean multi_conn_processes multi_conn_processes_clean udp_test upd_test_clean 
udp_test_executables:=udp_server udp_client
udp_server: udp_server.c
	-$(CC) -o $@ $^

udp_client: udp_client.c
	-$(CC) -o $@ $^

udp_test: $(udp_test_executables)

upd_test_clean:
	-rm ./$(firstword $(udp_test_executables)) ./$(lastword $(udp_test_executables))
socket_ipc_test: socket_ipc_test.c
	$(CC) -o $@ $^

socket_ipc_test_clean:
	-rm ./socket_ipc_test
