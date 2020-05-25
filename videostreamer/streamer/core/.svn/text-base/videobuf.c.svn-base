/*
 * videobuf.c
 *
 *  Created on: Jun 27, 2014
 *      Author: root
 */

/*
 ============================================================================
 Name        : vbufreader.c
 Author      : Zhangly
 Version     :
 Copyright   : WuhanJinsiDao Company limited!
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

/*
 ============================================================================
 Name        : vbuffer.c
 Author      : Zhangly
 Version     :
 Copyright   : WuhanJinsiDao Company limited!
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

/* A readers/writers program using a shared buffer and semaphores  */
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "videobuf.h"

#define buf_used 0     /* semaphore array index to check buffer elts used */
#define buf_space 1    /* semaphore array index to check buffer elts empty */


#define OPEN_FLAG O_RDWR | O_CREAT | O_TRUNC
#define OPEN_MODE 00777

#define IFLAGS (IPC_CREAT|IPC_EXCL)

void JSDpr_error(char *mess) {
	perror(mess);
	exit(1);
}


int JSDsem_init(int buffsize,int index) { /* procedure to create and initialize semaphores and return semaphore id,
 assuming two semaphores defined in the given array of semaphores     */
	int semid;

		/* create new semaphore set of 2 semaphores */

	if ((semid = semget(index, 2, 0600 | IFLAGS)) < 0)
	{
		if( errno == EEXIST)
		{
			semid=semget(index,2,0600|IPC_CREAT);
			if(semid <0)
				JSDpr_error("shmget");
			else
				return semid;
		}
		else
			JSDpr_error("shmget");
	}

	/* initialization of semaphores */
	/* BUF_SIZE free spaces in empty buffer */
	if (semctl(semid, buf_space, SETVAL, buffsize) < 0) {
		JSDpr_error("error in initializing first semaphore");
		exit(1);
	}

	/* 0 items in empty buffer */
	if (semctl(semid, buf_used, SETVAL, 0) < 0) {
		JSDpr_error("error in initializing second semaphore");
		exit(1);
	}
	return semid;
}

void JSDP(int semid, int index) {/* procedure to perform a P or wait operation on a semaphore of given index */
	struct sembuf sops[1]; /* only one semaphore operation to be executed */

	sops[0].sem_num = index;/* define operation on semaphore with given index */
	sops[0].sem_op = -1; /* subtract 1 to value for P operation */
	sops[0].sem_flg = 0; /* type "man semop" in shell window for details */

	if (semop(semid, sops, 1) == -1) {
		JSDpr_error("error in semaphore operation");
		exit(1);
	}
}

void JSDV(int semid, int index) {/* procedure to perform a V or signal operation on semaphore of given index */
	struct sembuf sops[1]; /* define operation on semaphore with given index */

	sops[0].sem_num = index;/* define operation on semaphore with given index */
	sops[0].sem_op = 1; /* add 1 to value for V operation */
	sops[0].sem_flg = 0; /* type "man semop" in shell window for details */

	if (semop(semid, sops, 1) == -1) {
		JSDpr_error("error in semaphore operation");
		exit(1);
	}
}

void init_audio_buffer(audiobuffer* abuf, char * filepath,int buffercnt,int index) {
	/* set up shared memory segment */
	int result;
	/* Open a file for writing.
	 *  - Creating the file if it doesn't exist.
	 *  - Truncating it to 0 size if it already exists. (not really needed)
	 *
	 * Note: "O_WRONLY" mode is not sufficient when mmaping.
	 */

	//vbuf->fd = shm_open(filepath, OPEN_FLAG, OPEN_MODE);
	abuf->fd = open(filepath, OPEN_FLAG, OPEN_MODE);
	printf("fd is %d\n",abuf->fd);

	if (abuf->fd == -1) {
		JSDpr_error("Error opening file for writing");
		exit(EXIT_FAILURE);
	}
	abuf->buffercnt = buffercnt;
	/* Stretch the file size to the size of the (mmapped) array of ints
	 */
	abuf->size = (abuf->buffercnt + 2) * sizeof(videobitstream);
	//调整确定文件共享内存的空间

	result = ftruncate(abuf->fd, abuf->size);
	if (-1 == result)
	{
		JSDpr_error("ftruncate faile: ");
		exit(1);
	}
	abuf->shared_memory = mmap(NULL, abuf->size, PROT_READ | PROT_WRITE,
	 MAP_SHARED, abuf->fd, 0);

	if (abuf->shared_memory == (caddr_t) -1) {
		JSDpr_error("error in mmap while allocating shared memory\n");
		exit(1);
	}

	/* set up pointers to appropriate places in shared memory segment */
	abuf->buffer = (videobitstream*) abuf->shared_memory; /* logical buffer starts at shared segment */
	/* create and initialize semaphore */
	abuf->semid = JSDsem_init(abuf->buffercnt,index);
	printf("semid is %d\n",abuf->semid );

	abuf->in = 0; /* initial starting points */
	abuf->out = 0;
}

void init_video_buffer(videobuffer* vbuf, char * filepath,int buffercnt,int index) {
	/* set up shared memory segment */
	int result;
	/* Open a file for writing.
	 *  - Creating the file if it doesn't exist.
	 *  - Truncating it to 0 size if it already exists. (not really needed)
	 *
	 * Note: "O_WRONLY" mode is not sufficient when mmaping.
	 */

	//vbuf->fd = shm_open(filepath, OPEN_FLAG, OPEN_MODE);
	vbuf->fd = open(filepath, OPEN_FLAG, OPEN_MODE);
	printf("fd is %d\n",vbuf->fd);

	if (vbuf->fd == -1) {
		JSDpr_error("Error opening file for writing");
		exit(EXIT_FAILURE);
	}
	vbuf->buffercnt = buffercnt;
	/* Stretch the file size to the size of the (mmapped) array of ints
	 */
	vbuf->size = (vbuf->buffercnt + 2) * sizeof(videobitstream);
	//调整确定文件共享内存的空间

	result = ftruncate(vbuf->fd, vbuf->size);
	if (-1 == result)
	{
		JSDpr_error("ftruncate faile: ");
		exit(1);
	}
	vbuf->shared_memory = mmap(NULL, vbuf->size, PROT_READ | PROT_WRITE,
	 MAP_SHARED, vbuf->fd, 0);

	if (vbuf->shared_memory == (caddr_t) -1) {
		JSDpr_error("error in mmap while allocating shared memory\n");
		exit(1);
	}

	/* set up pointers to appropriate places in shared memory segment */
	vbuf->buffer = (videobitstream*) vbuf->shared_memory; /* logical buffer starts at shared segment */
	/* create and initialize semaphore */
	vbuf->semid = JSDsem_init(vbuf->buffercnt,index);
	printf("semid is %d\n",vbuf->semid );

	vbuf->in = 0; /* initial starting points */
	vbuf->out = 0;

}
void audiobuffer_read(audiobuffer* abuf, audiobitstream* value){
	JSDP(abuf->semid, buf_used); /* wait semaphore for something used */
	*value = abuf->buffer[abuf->out];
	abuf->out = (abuf->out + 1) % abuf->buffercnt;
//	printf("Reader's report: item %2d == %2d\n", vbuf->out , value->videoseq);
	JSDV(abuf->semid, buf_space); /* signal semaphore for space available */
}


void videobuffer_read(videobuffer* vbuf, videobitstream* value) {
	JSDP(vbuf->semid, buf_used); /* wait semaphore for something used */
	*value = vbuf->buffer[vbuf->out];
	vbuf->out = (vbuf->out + 1) % vbuf->buffercnt;
//	printf("Reader's report: item %2d == %2d\n", vbuf->out , value->videoseq);
	JSDV(vbuf->semid, buf_space); /* signal semaphore for space available */
}
void audiobuffer_write(audiobuffer* abuf, audiobitstream value){
	JSDP(abuf->semid, buf_space);/* wait semaphore for space available */

	abuf->buffer[abuf->in] = value; /* put data in buffer */
	abuf->in = (abuf->in + 1) % abuf->buffercnt;
	//printf("Writer's report: item %2d put in buffer\n", j_child);
	JSDV(abuf->semid, buf_used); /* signal semaphore for something used */
}

void videobuffer_write(videobuffer* vbuf, videobitstream value) {
	JSDP(vbuf->semid, buf_space);/* wait semaphore for space available */

	vbuf->buffer[vbuf->in] = value; /* put data in buffer */
	vbuf->in = (vbuf->in + 1) % vbuf->buffercnt;
	//printf("Writer's report: item %2d put in buffer\n", j_child);
	JSDV(vbuf->semid, buf_used); /* signal semaphore for something used */

}
void audiobuffer_release(audiobuffer* abuf){
	if (semctl(abuf->semid, 0, IPC_RMID) < 0) {
		JSDpr_error("error in removing semaphore from the system");
		exit(1);
	}
	/* Don't forget to free the mmapped memory
	 */
	if (munmap(abuf->shared_memory, abuf->size) == -1) {
		JSDpr_error("Error un-mmapping the file");
		/* Decide here whether to close(fd) and exit() or not. Depends... */
	}

	/* Un-mmaping doesn't close the file, so we still need to do that.
	 */
	close(abuf->fd);

}
void videobuffer_release(videobuffer* vbuf) {
	if (semctl(vbuf->semid, 0, IPC_RMID) < 0) {
		JSDpr_error("error in removing semaphore from the system");
		exit(1);
	}
	/* Don't forget to free the mmapped memory
	 */
	if (munmap(vbuf->shared_memory, vbuf->size) == -1) {
		JSDpr_error("Error un-mmapping the file");
		/* Decide here whether to close(fd) and exit() or not. Depends... */
	}

	/* Un-mmaping doesn't close the file, so we still need to do that.
	 */
	close(vbuf->fd);
}
void audiobuffer_free(audiobuffer* abuf){
	/* Don't forget to free the mmapped memory
	 */
	if (munmap(abuf->shared_memory, abuf->size) == -1) {
		JSDpr_error("Error un-mmapping the file");
		/* Decide here whether to close(fd) and exit() or not. Depends... */
	}
	/* Un-mmaping doesn't close the file, so we still need to do that.
	 */
	close(abuf->fd);
}

void videobuffer_free(videobuffer* vbuf)
{

	/* Don't forget to free the mmapped memory
	 */
	if (munmap(vbuf->shared_memory, vbuf->size) == -1) {
		JSDpr_error("Error un-mmapping the file");
		/* Decide here whether to close(fd) and exit() or not. Depends... */
	}

	/* Un-mmaping doesn't close the file, so we still need to do that.
	 */
	close(vbuf->fd);
}
////init for video buffer
//videobuffer m_videobuffer[]={NULL,NULL,NULL,NULL};
//
//void init_videoBuffer()
//{
//	int i=0;
//	init_video_buffer(&m_videobuffer[0], "/lib/tmvideobuf0",3);
//	init_video_buffer(&m_videobuffer[1], "/lib/tmvideobuf1",3);
//	init_video_buffer(&m_videobuffer[2], "/lib/tmvideobuf2",3);
//	init_video_buffer(&m_videobuffer[3], "/lib/tmvideobuf0",3);
//}
//
//void release_videoBuffer(int index)
//{
//	videobuffer_release(&m_videobuffer[index]);
//}
//void release_allvideoBuffer( )
//{
//	videobuffer_release(&m_videobuffer[0]);
//	videobuffer_release(&m_videobuffer[1]);
//	videobuffer_release(&m_videobuffer[2]);
//	videobuffer_release(&m_videobuffer[3]);
//}
//
//void free_videoBuffer(int index){
//	videobuffer_free(&m_videobuffer[index]);
//}
//
