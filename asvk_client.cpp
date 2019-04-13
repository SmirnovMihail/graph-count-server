#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>


enum size{bufsize=128};


class Buffer
{
  int busy;
  int size;
  char *content;
  void extend();
public:
  Buffer();
  void initialize();
  void add_char(char c);
  void add_string(char *str);
  int get_busy();
  char *get_content();
  char get_char(int index);
  void cleaning();
  void update_info();
  char *string_search();
  ~Buffer();
};


class Server
{
  int fd;
  Buffer buf;
public: 
  Server(char **argv);
  void send_data(char *buf);
  void receive_data();
};


Buffer :: Buffer()
{
  initialize();
}


void Buffer :: initialize()
{
  busy=0;
  size=8;
  content=new char[size+1];
  content[size]=0;
}


void Buffer :: add_char(char c)
{
  if (busy>=size)
    extend();
  content[busy]=c;
  busy++;
  content[busy]=0;
}  


void Buffer :: add_string(char *str)
{
  int len;
  len=strlen(str);
  while (busy+len>=size)
    extend();
  for (int i=0; i<len; i++)
    content[busy+i]=str[i];
  busy=busy+len;
  content[busy]=0;
}


void Buffer :: extend()
{
  char *new_buf=new char[size*2+1];
  for (int i=0; i<busy; i++)
    new_buf[i]=content[i];
  delete[] content;
  content=new_buf;
  size=size*2;
}


int Buffer :: get_busy()
{
  return busy;
}


char* Buffer :: get_content()
{
  return content;
}


char Buffer :: get_char(int index)
{
  return content[index];
}


void Buffer :: cleaning()
{
  delete[] content;
  initialize();
}


void Buffer :: update_info()
{
  content=new char[size];
  busy=0;
}


char* Buffer :: string_search()
{
  for (int i=0; i<busy; i++)
  {
    if (content[i]=='\n')
    {
      char* string=new char[i];
      for (int j=0; j<i; j++)
        string[j]=content[j];
      string[i]=0;
      busy=busy-i-1;
      char* new_cont=new char[busy];
      for (int j=0; j<busy; j++)
        new_cont[j]=content[j+i+1];
      delete[] content;
      content=new_cont;
      return string;
    }
  }
  return 0;
}


Buffer :: ~Buffer()
{
  delete[] content;
}


Server :: Server(char **argv)
{
  struct sockaddr_in addr;
  addr.sin_family=AF_INET;  
  addr.sin_port=htons(atoi(argv[2]));
  if (inet_aton(argv[1], &(addr.sin_addr))==0)
  {
    perror("Invalid IP address\n");
    exit(1);
  }
  fd=socket(AF_INET, SOCK_STREAM, 0);
  if (fd==-1)
  {
    perror("Can't create socket\n");
    exit(1);
  }
  if (connect(fd, (struct sockaddr *) &addr, sizeof(addr))!=0)
  {
    perror("Can't connect to server\n");
    exit(1);
  }
}


char *word_selection(char *string, int number)
{
  char *word;
  int i=0, count=0, f=0, len=0, j=0;
  while (string[i]!=0)
  { 
    if (count==number-1 && f==0)  
    {
      while (string[i]==' ' || string[i]=='\n')
        i++;
      while (string[len+i]!=' ' && string[len+i]!='\n' &&
             string[len+i]!=0)
        len++;
      word=new char[len+1];
      for (j=0; j<len; j++)
        word[j]=string[i+j];
      word[j]=0;
      return word;
    } 
    if (string[i]!=' ' && string[i]!='\n' && f==0)
    {
      f=1;
      count++;
    }
    if ((string[i]==' ' || string[i]=='\n') && f==1)
    {
      f=0;
    }
    i++;
  }
  return 0;
}


void Server :: send_data(char *buf)
{
  char end[]=" end\n", exit[]="exit", 
       mes[]="Enter two vertexes to count shortest path or print exit\n";
  Buffer buffer;
  char tmp_buf[bufsize+1];
  char *string;
  int len, flag=0;
  write(fd, buf, strlen(buf)); 
  printf("%s", mes);
  while (flag==0 && (len=read(0, tmp_buf, bufsize))!=0)
  {
   buffer.add_string(tmp_buf);
    if ((string=buffer.string_search())!=0)
    {
      if (strcmp(string, exit)==0)
      {
        printf("Goodbye\n");
        flag=1;
      }
      else
      {
        if (write(fd, string, strlen(string))==-1)
        {
          printf("Sorry, but connection to server lost\n");
          flag=1;
        }
        write(fd, end, strlen(end));
        receive_data();
        buffer.cleaning();
        printf("%s", mes);
      }
      delete[] string;
    } 
  }
}


void Server :: receive_data()
{
  char tmp_buf[bufsize];
  //char end[]="end";
  int len, flag=0;
  char *string;
  while(flag==0 && (len=read(fd, tmp_buf, bufsize))>0)
  {
    buf.add_string(tmp_buf);
    while ((string=buf.string_search())!=0)
    {
      /*printf("'%s' '%s'\n", string, end); 
      if (strcmp(string, end)==0)
        flag=1;
      else*/
        flag=1;
        printf("Shortest path=%s\n", string);
      delete[] string;
    }
  }
  buf.cleaning();
  if (len<0)
        {
          printf("Sorry, but connection to server lost\n");
        }
} 

 
Buffer open_file_and_read_info()
{ 
  Buffer buf;
  int flag=0;
  char tmp_buf[bufsize];
  int fd, len;
  while (flag==0)
  {
    printf("Please enter filename:\n");
    scanf("%s", tmp_buf);
    if ((fd=open(tmp_buf, O_RDONLY))==-1)
    {
      fprintf(stderr, ">>>Can't open flie %s\n", tmp_buf);
    }
    else 
      flag=1;
  }
  while ((len=read(fd, tmp_buf, bufsize))>0)
  {
    tmp_buf[len]=0;
    buf.add_string(tmp_buf);
  }
  printf("\nInput graph:\n%s\n", buf.get_content());
  close(fd);
  return buf;
}


int main(int argc, char **argv)
{
  char *data;
  Buffer buf=open_file_and_read_info();
  if (argc<2)
  {
    perror("You didn't entered ip and port\n");
  } else
  if ((data=buf.get_content()))
  {
    Server client(argv);
    client.send_data(data);
  } else
    perror("File is emplty\n");
  return 0;
}
