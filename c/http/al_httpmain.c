
// TODO

{

  // for init see net/connect.c
  
  al_http_get_resolved_address(host,LOOPBACKSERV_PORT,&connection);

  printf("al_http_client_transaction_with_server\n");
  al_http_client_transaction_with_server(&httpcontext, &toserver, &connection);

}
