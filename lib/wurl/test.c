#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //for exit(0);
#include<errno.h> //For errno - the error number
#include<netdb.h> //hostent
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <signal.h>
#include <dirent.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <errno.h>
#include <ifaddrs.h>
#include <sys/stat.h>
#include <libgen.h>
	
#define CLIENT_NAME "wurl"
#define CLIENT_HOST "192.168.9.249"
#define CONN_TIME_OUT_S 3
#define MAX_BUFF 1024
#define REQUEST_BOUNDARY "------wURLFormBoundaryVo4QYVaSVseFNpeK"
#define GET 0
#define POST 1
typedef struct{
	int type; 		// POST(1) or GET(0)
	char* resource;	// path
	char* ctype;	// content type, used by POST
	int clen;		// content length, used by POST
	unsigned char* data ;
} wurl_header_t;

/*get the ip by hostname*/
int wurl_ip_from_hostname(const char * hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;    
    if ( (he = gethostbyname( hostname ) ) == NULL) 
    {
        // get the host info
        herror("gethostbyname");
        return -1;
    }
    addr_list = (struct in_addr **) he->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }
    return -1;
}

/*
send a request
*/
int wurl_request_socket(const char* ip, int port)
{
	int sockfd, bytes_read;
	struct sockaddr_in dest;
	
	// time out setting
	struct timeval timeout;      
	timeout.tv_sec = CONN_TIME_OUT_S;
	timeout.tv_usec = 0;
	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror("Socket");
		return -1;
	}
	if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
	        perror("setsockopt failed\n");

    if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
        perror("setsockopt failed\n");
	
    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    if ( inet_aton(ip, &dest.sin_addr.s_addr) == 0 )
    {
		perror(ip);
		close(sockfd);
		return -1;
    }
	if ( connect(sockfd, (struct sockaddr*)&dest, sizeof(dest)) != 0 )
	{
		close(sockfd);
		perror("Connect");
		return -1;
	}
	return sockfd;
}
/*
POST %s HTTP/1.0\r\n
Host: %s\r\n
User-Agent: %s\r\n
Content-Type: %s\r\n
Content-Length: %d\r\n\r\n"

maybe cookie support? this can cause security problem

GET %s HTTP/1.0\r\n
Host: %s\r\n
User-Agent: %s\r\n\r\n"

multipart
POST %s HTTP/1.1
Host: %s
User-Agent: %s
Content-Type: multipart/form-data; boundary=----------287032381131322
Content-Length: %d

------------287032381131322
Content-Disposition: form-data; name="datafile1"; filename="r.gif"
Content-Type: image/gif

GIF87a.............,...........D..;
------------287032381131322
Content-Disposition: form-data; name="datafile2"; filename="g.gif"
Content-Type: image/gif

GIF87a.............,...........D..;
------------287032381131322
Content-Disposition: form-data; name="datafile3"; filename="b.gif"
Content-Type: image/gif

GIF87a.............,...........D..;
------------287032381131322--
*/

int wurl_header(int sockfd, wurl_header_t rq)
{	
	char buff[MAX_BUFF];
	if(sockfd < 0) return -1;
	
	if(rq.type == GET)  // GET
	{
		send(sockfd,"GET ",4,0);
		send(sockfd,rq.resource, strlen(rq.resource),0);
		send(sockfd," HTTP/1.0\r\n",11,0);
	}
	else
	{
		send(sockfd,"POST ",5, 0);
		send(sockfd,rq.resource, strlen(rq.resource),0);
		send(sockfd," HTTP/1.0\r\n",11,0);
		sprintf(buff,"Content-Type: %s\r\n", rq.ctype);
		send(sockfd,buff,strlen(buff),0);
		sprintf(buff,"Content-Length: %d\r\n", rq.clen);
		send(sockfd,buff,strlen(buff),0);
	}
	// host dont need to send the host
	//sprintf(buff,"Host: %s\r\n",CLIENT_HOST);
	//send(sockfd,buff,strlen(buff),0);
	// user agent
	sprintf(buff,"User-Agent: %s\r\n",CLIENT_NAME);
	send(sockfd,buff,strlen(buff),0);
	// terminate  request
	send(sockfd,"\r\n",2,0);

	// if there is data, send out
	if(rq.type == POST && rq.data)
	{
		send(sockfd,rq.data,rq.clen,0);
	}
	return 0;
}


// this will be removed
#define IEQU(a,b) (strcasecmp(a,b) == 0)
#define LOG(x) printf(x)
/**
 * Trim a string by a character on both ends
 * @param str   The target string
 * @param delim the delim
 */
void trim(char* str, const char delim)
{
    char * p = str;
    int l = strlen(p);

    while(p[l - 1] == delim) p[--l] = 0;
    while(* p && (* p) == delim ) ++p, --l;
    memmove(str, p, l + 1);
}
/**
 * Get extension of a file name
 * @param  file The file name
 * @return      the extension
 */
char* ext(const char* file)
{
	char* token,*ltoken = "";
	if(file == NULL) return NULL;
	char* str_cpy = strdup(file);
	if(strstr(str_cpy,".")<= 0) return "";
	if(*file == '.')
		file++;

	while((token = strsep(&str_cpy,".")) && strlen(token)>0) {ltoken = token;}
	free(str_cpy);
	return ltoken;

}
/**
 * Get correct HTTP mime type of a file
 * This is a minimalistic mime type list supported
 * by the server
 * @param  file File name
 * @return      The HTTP Mime Type
 */
char* mime(const char* file)
{
	char * ex = ext(file);
	if(IEQU(ex,"bmp"))
		return "image/bmp";
	else if(IEQU(ex,"jpg") || IEQU(ex,"jpeg"))
		return "image/jpeg";
	else if(IEQU(ex,"css"))
		return "text/css";
	else if(IEQU(ex,"csv"))
		return "text/csv";
	else if(IEQU(ex,"pdf"))
		return "application/pdf";
	else if(IEQU(ex,"gif"))
		return "image/gif";
	else if(IEQU(ex,"html")||(IEQU(ex,"htm")))
		return "text/html";
	else if(IEQU(ex,"json"))
		return "application/json";
	else if(IEQU(ex,"js"))
		return "application/javascript";
	else if(IEQU(ex,"png"))
		return "image/png";
	else if(IEQU(ex,"ppm"))
		return "image/x-portable-pixmap";
	else if(IEQU(ex,"rar"))
		return "application/x-rar-compressed";
	else if(IEQU(ex,"tiff"))
		return "image/tiff";
	else if(IEQU(ex,"tar"))
		return "application/x-tar";
	else if(IEQU(ex,"txt"))
		return "text/plain";
	else if(IEQU(ex,"ttf"))
		return "application/x-font-ttf";
	else if(IEQU(ex,"xhtml"))
		return "application/xhtml+xml";
	else if(IEQU(ex,"xml"))
		return "application/xml";
	else if(IEQU(ex,"zip"))
		return "application/zip";
	else if(IEQU(ex,"svg"))
		return "image/svg+xml";
	else if(IEQU(ex,"eot"))
		return "application/vnd.ms-fontobject";
	else if(IEQU(ex,"woff") || IEQU(ex,"woff2"))
		return "application/x-font-woff";
	else if(IEQU(ex,"otf"))
		return "application/x-font-otf";
	//audio
	else if(IEQU(ex,"mp3"))
		return "audio/mpeg";
	else 
		// The other type will be undestant as binary
		return "application/octet-stream";
}

void wurl_send_files(int sockfd,char* resource, int n, char* name [], char* files[])
{
	char buff[MAX_BUFF];
	wurl_header_t rq;
	rq.type = POST;
	rq.resource = resource;
	// get the total size of data
	int totalsize = 0;
	FILE* fd;
	struct stat st;
	for(int i = 0; i < n; ++i)
	{
		if(stat(files[i], &st) != 0) continue;
		totalsize += st.st_size;
	}
	rq.clen = totalsize;
	sprintf(buff,"%s; boundary=%s","multipart/form-data",REQUEST_BOUNDARY);
	rq.ctype = buff;
	
	// now send the header
	wurl_header(sockfd,rq);
	// now send the files
	size_t size;
	for(int i = 0; i < n; ++i)
	{
		fd = fopen(files[i],"rb");
		if(!fd) continue;
		// first send the boundary
		sprintf(buff,"%s\r\n",REQUEST_BOUNDARY);
		send(sockfd, buff, strlen(buff),0);
		// content disposition
		sprintf(buff,"Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n",
					name[i],basename(files[i]));
		send(sockfd,buff, strlen(buff),0);
		// content type
		sprintf(buff,"Content-Type: %s\r\n\r\n",mime(files[i]));
		send(sockfd,buff, strlen(buff),0);
		// now send the file
		while(!feof(fd))
		{
			size = fread(buff,1,MAX_BUFF,fd);
			send(sockfd,buff,size,0);
			//if(!__b(client,buffer,size)) return;
		}
		fclose(fd);
		send(sockfd,"\r\n",2,0);
	}
	//end the boudary
	sprintf(buff,"%s--\n",REQUEST_BOUNDARY);
	send(sockfd,buff,strlen(buff),0);
}

/**
 * Read the socket request in to a buffer or size
 * The data is read until the buffer is full or
 * there are a carrier return character
 * @param  sock socket
 * @param  buf  buffer
 * @param  size size of buffer
 * @return      number of bytes read
 */
int wurl_read_buf(int sock, char*buf,int size)
{
	int i = 0;
	char c = '\0';
	int n;
	while ((i < size - 1) && (c != '\n'))
	{
		n = recv(sock, &c, 1, 0);
		if (n > 0)
		{
			buf[i] = c;
			i++;
		}
		else
			c = '\n';
	}
	buf[i] = '\0';
	return i;
}
/*
	POST example
	wurl_header_t rq;
	rq.resource = path;
	rq.type = POST;
	rq.data = "s=a&q=b#test";//"{\"name\":\"sang\"}";
	rq.clen =  strlen(rq.data);
	rq.ctype = "application/x-www-form-urlencoded";//"application/json";
	
	if(wurl_request(hostname,port,&rq,1) == 0)
	{
		printf(rq.data);
	}
	else
	{
		printf("Cannot connect to host\n");
	}
	
	DOWNLOAD
	wurl_header_t rq;
	rq.resource = path;
	rq.type = GET;
	
	if(wurl_download(hostname,port,&rq,file) == 0)
	{
		printf("Download sucess ful\n");
	}
	else
	{
		printf("Cannot connect to host\n");
	}

	
	upload example
		// send files
		char * names[2];
		names[0] = "zip";
		names[1] = "text";
		char* files[2];
		files[0] = "/Users/mrsang/tmp/Archive.zip";
		files[1] = "/Users/mrsang/tmp/test.py";
		
		wurl_send_files(sock,path,2,names, files);
		printf("RETURN:\n");
		size = wurl_read_buf(sock,buff, MAX_BUFF);
		while(size > 0)
		{
			printf("%s", buff);
			size = wurl_read_buf(sock,buff, MAX_BUFF);
		}
		close(sock);
*/

/*
	hostname
	port
	header for request and respond
	lazy : 	if 1, all data is read to the header
			if 0, user has the responsibility to handler it
*/
int wurl_request(const char* hostname, int port, wurl_header_t* header, int lazy)
{
	char ip[100];
	char buff[MAX_BUFF];
	wurl_ip_from_hostname(hostname ,ip);
	int sock = wurl_request_socket(ip, port);
	
	if(sock <= 0) return -1;
	// send header
	wurl_header(sock,*header);
	// read respond header
	int size  = wurl_read_buf(sock,buff,MAX_BUFF);
	char* token;
	while (size > 0 && strcmp("\r\n",buff))
	{
		char* line = strdup(buff);
		token = strsep(&line,":");
		trim(token,' ');
		if(token != NULL &&strcasecmp(token,"Content-Type") == 0)
		{
			header->ctype = strsep(&line,":");
			trim(header->ctype,' ');
			trim(header->ctype,'\n');
			trim(header->ctype,'\r');
		} else if(token != NULL &&strcasecmp(token,"Content-Length") == 0)
		{
			token = strsep(&line,":");
			trim(token,' ');
			header->clen = atoi(token);
		}
		//if(line) free(line);
		size  = wurl_read_buf(sock,buff,MAX_BUFF);
	}
	if(header->ctype == NULL || header->clen == -1)
	{
		LOG("Bad data\n");
		return -1;
	}
	
	// read data if lazy
	if(lazy)
	{
		// read line by line, ignore content length
		int total_length = 0;
		char* tmp = NULL;
		int CHUNK = 512;
		header->data = (unsigned char*) malloc(CHUNK);
		int cursize = CHUNK;
		int size = wurl_read_buf(sock,buff,MAX_BUFF);
		while(size > 0)
		{
			if(total_length+size > cursize)
			{
				tmp = (unsigned char*) realloc(header->data,total_length + size);
				if(!tmp) 
				{
					if(header->data) free(header->data);
					break;
				}
				header->data = tmp;
			}
			memcpy(header->data+total_length,buff,size);
			total_length += size;
			size = wurl_read_buf(sock,buff,MAX_BUFF);
		}
		header->clen = total_length;
		close(sock);
		return 0;
	} 	
	return sock;
}

int wurl_download(const char* hostname, int port, wurl_header_t* h, const char* to)
{
	// we will handler the data reading
	int sock = wurl_request(hostname, port,h,0);
	unsigned char buff[MAX_BUFF];
	if(sock < 0) return -1;
	
	FILE* fp =  fopen(to,"wb");
	int size;
	if(fp)
	{
		while((size = wurl_read_buf(sock,buff, MAX_BUFF)) > 0)
		{
			fwrite(buff, size, 1, fp);
		}
		fclose(fp);
	}
	else
	{
		close(sock);
		return -1;
	}
	close(sock);
	return 0;
}

int main (int argc, char const *argv[])
{
	if(argc < 4)
	{
	        printf("wurl [host] [port] [path]\n");
	        exit(1);
	}
     
	char *hostname = argv[1];
	char* path = argv[3];	
	int port = atoi(argv[2]);
	char*file = argv[4];
	wurl_header_t rq;
	rq.resource = path;
	rq.type = POST;
	rq.data = "s=a&q=b#test";//"{\"name\":\"sang\"}";
	rq.clen =  strlen(rq.data);
	rq.ctype = "application/x-www-form-urlencoded";//"application/json";
	
	if(wurl_download(hostname,port,&rq,file) == 0)
	{
		printf("Download sucess ful\n");
	}
	else
	{
		printf("Cannot connect to host\n");
	}
	return 0;
}