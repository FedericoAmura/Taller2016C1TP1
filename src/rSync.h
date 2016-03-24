/*
 * rSync.h
 *
 *  Created on: Mar 24, 2016
 *      Author: freddy
 */

#ifndef RSYNC_H_
#define RSYNC_H_

// Funcion que inicia un servidor donde puede actualizarse un file
int startServerAndWaitRequestForFile(char* port);

// Funcion que inicia una conexion a un servidor y solicita las actualizaciones de un file local respecto al suyo
int requestFileFromServer(char* hostname, char* port, char* old_local_file, char* new_local_file, char* new_remote_file, char* block_size);

#endif /* RSYNC_H_ */
