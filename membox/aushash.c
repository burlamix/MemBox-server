void free_message(void * mex){
	message_t*= *((message_t*)mex);
	free(mex->hdr);
	free(mex->data->buf);
	free(mex->data);
	free(mex);

}