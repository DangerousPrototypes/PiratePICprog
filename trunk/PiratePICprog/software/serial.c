/*
 * OS independent serial interface
 *
 * Heavily based on Pirate-Loader:
 * http://the-bus-pirate.googlecode.com/svn/trunk/bootloader-v4/pirate-loader/source/pirate-loader.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <string.h>

#include "serial.h"
extern int disable_comport;

int serial_setup(int fd, speed_t speed)
{
#ifdef WIN32
	COMMTIMEOUTS timeouts;
	DCB dcb = {0};
	HANDLE hCom = (HANDLE)fd;

	dcb.DCBlength = sizeof(dcb);

	dcb.BaudRate = speed;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;

	if( !SetCommState(hCom, &dcb) ){
		return -1;
	}


	timeouts.ReadIntervalTimeout = 100;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.ReadTotalTimeoutConstant = 100;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 100;

	if (!SetCommTimeouts(hCom, &timeouts)) {
		return -1;
	}

	return 0;
#else
	struct termios t_opt;

	/* set the serial port parameters */
	fcntl(fd, F_SETFL, 0);
	tcgetattr(fd, &t_opt);
	cfsetispeed(&t_opt, speed);
	cfsetospeed(&t_opt, speed);
	t_opt.c_cflag |= (CLOCAL | CREAD);
	t_opt.c_cflag &= ~PARENB;
	t_opt.c_cflag &= ~CSTOPB;
	t_opt.c_cflag &= ~CSIZE;
	t_opt.c_cflag |= CS8;
	t_opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	t_opt.c_iflag &= ~(IXON | IXOFF | IXANY);
	t_opt.c_oflag &= ~OPOST;
	t_opt.c_cc[VMIN] = 0;
	t_opt.c_cc[VTIME] = 10;
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &t_opt);
#endif
	return 0;
}

int serial_write(int fd, char *buf, int size)
{
	int ret = 0;
#ifdef WIN32
	HANDLE hCom = (HANDLE)fd;
	int res = 0;
	unsigned long bwritten = 0;


	res = WriteFile(hCom, buf, size, &bwritten, NULL);

	if( res == FALSE ) {
		ret = -1;
	} else {
		ret = bwritten;
	}
#else
	ret = write(fd, buf, size);
#endif

	//fprintf(stderr, "size = %d ret = %d\n", size, ret);
	//buspirate_print_buffer(buf, size);
    if (disable_comport !=1)   //added for no port
    {


	if (ret != size)
		fprintf(stderr, "Error sending data");
	return ret;
    }
    return ret;
}

int serial_read(int fd, char *buf, int size)
{
	int len = 0;
	int ret = 0;
#ifndef WIN32
	int timeout = 0;
#endif
#ifdef WIN32
	HANDLE hCom = (HANDLE)fd;
	unsigned long bread = 0;

	ret = ReadFile(hCom, buf, size, &bread, NULL);

	if( ret == FALSE || ret==-1 ) {
		len= -1;
	} else {
		len=bread;
	}

#else
	while (len < size) {
		ret = read(fd, buf+len, size-len);
		if (ret == -1){
			//printf("ret -1");
			return -1;
		}

		if (ret == 0) {
			timeout++;

			if (timeout >= 10)
				break;

			continue;
		}

		len += ret;
	}
#endif
	//printf("should have read = %i actual size = %i \n", size, len);
	//fprintf(stderr, "should have read = %d actual size = %d \n", size, len);
	//buspirate_print_buffer(buf, len);
if (disable_comport!=1)
{


	if (len != size)
		fprintf(stderr, "Error sending data");


	return len;
}
else
return len;
}

int serial_open(char *port)
{
	int fd;
#ifdef WIN32
	static char full_path[32] = {0};

	HANDLE hCom = NULL;

	if( port[0] != '\\' ) {
		_snprintf(full_path, sizeof(full_path) - 1, "\\\\.\\%s", port);
		port = full_path;
	}

	hCom = CreateFileA(port, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if( !hCom || hCom == INVALID_HANDLE_VALUE ) {
		fd = -1;
	} else {
		fd = (int)hCom;
	}
#else
	fd = open(port, O_RDWR | O_NOCTTY);
	if (fd == -1) {
		fprintf(stderr, "Could not open serial port.");
		return -1;
	}
#endif
	return fd;
}

int serial_close(int fd)
{
#ifdef WIN32
	HANDLE hCom = (HANDLE)fd;

	CloseHandle(hCom);
#else
	close(fd);
#endif
	return 0;
}

/*
int readWithTimeout(int fd, uint8_t *out, int length, int timeout)
{
	fd_set fds;
	struct timeval tv = {timeout, 0};
	int res = -1;
	int got = 0;

	do {

		FD_ZERO(&fds);
		FD_SET(fd, &fds);

		res = select(fd + 1, &fds, NULL, NULL, &tv);

		if( res > 0 ) {
			res = read(fd, out, length);
			if( res > 0 ) {
				length -= res;
				got    += res;
				out    += res;
			} else {
				break;
			}
		} else {
			return res;
		}
	} while( length > 0);

	return got;
}

int configurePort(int fd, unsigned long baudrate)
{
#ifdef WIN32
	COMMTIMEOUTS timeouts;
	DCB dcb = {0};
	HANDLE hCom = (HANDLE)fd;

	dcb.DCBlength = sizeof(dcb);

	dcb.BaudRate = baudrate;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;

	if( !SetCommState(hCom, &dcb) ){
		return -1;
	}


	timeouts.ReadIntervalTimeout = 100;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.ReadTotalTimeoutConstant = 100;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 100;

	if (!SetCommTimeouts(hCom, &timeouts)) {
		return -1;
	}

	return (int)hCom;
#else
    speed_t baud = B921600;
	struct termios g_new_tio;

	switch (baudrate) {
		case 921600:
			baud = B921600;
			break;
		case 115200:
			baud = B115200;
			break;
		case 1000000:
			baud = B1000000;
			break;
		case 1500000:
			baud = B1500000;
		default:
			printf("unknown speed setting \n");
			return -1;
			break;
	}

	memset(&g_new_tio, 0x00 , sizeof(g_new_tio));
	cfmakeraw(&g_new_tio);

	g_new_tio.c_cflag |=  (CS8 | CLOCAL | CREAD);
	g_new_tio.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
	g_new_tio.c_oflag = 0;
	g_new_tio.c_lflag = 0;

	g_new_tio.c_cc[VTIME] = 0;
	g_new_tio.c_cc[VMIN] = 1;

#ifdef MACOSX

	if( tcsetattr(fd, TCSANOW, &g_new_tio) < 0 ) {
		return -1;
	}

	return ioctl( fd, IOSSIOSPEED, &baud );
#else
	cfsetispeed (&g_new_tio, baudrate);
	cfsetospeed (&g_new_tio, baudrate);

	tcflush(fd, TCIOFLUSH);

	return tcsetattr(fd, TCSANOW, &g_new_tio);
#endif //#ifdef MACOSX

#endif
}

int openPort(const char* dev, unsigned long flags)
{
	return open(dev, O_RDWR | O_NOCTTY | O_NDELAY | flags);
}
*/
