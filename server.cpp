#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>


enum bs {bufsize=8};


struct Edge_list
{
  int start_vertex;
  int finish_vertex;
  int weight;
  Edge_list *next;
};


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
//  ~Buffer();
};


struct Client_list
{
  int socket_fd;
  struct Client_list *next;
  Buffer buf;
  Edge_list *edges;
};


struct Connected_vertexes
{
  int num;
  int weight;
  Connected_vertexes *next;
};


struct Vertex_list
{
  int num, h, virt_num, visited, shortest_path;
  Connected_vertexes *vertexes;
  Vertex_list *next;
  void add_vertex(int num, int weight);
};


class Graph
{
  int max_vert, max_weight, edges_num, vertex_num, set;
  int **A;
  Vertex_list *list;
  Vertex_list* search_vertex(int num) const;
  void max_vert_and_weight_count();
  void edges_and_verts_count();
  int h(int num);
  int virt(int num);
  int check_length(Connected_vertexes *vertex, Vertex_list *ptr);
  void step(int v1, int v2);
  int check_for_visiting(int num);
  void set_length(int num);
public:
  void set_graph(Edge_list *edges);
  void print_graph();
  void add_special_vertex();
  void bellman_f();
  void reweight();
  int dijkstra(int v1, int v2);
  //void weight(int vert1, int vert2)
};


class Server
{
  int ls, max_d, flag;
  fd_set readfds;
  Client_list *list;
  Graph graph;
public:
  Server();
  void set_formation();
  void add_socket_fd(int socket);
  void creation_of_listen_socket(char **argv);
  void wait_for_activity();
  void acception_of_client(int ls);
  void add_edge(char *string);
  void add_socket(int socket);
  void delete_socket(int socket);
  void receiving_data();
  void process_information(char *string, Client_list *tmp);
};


void Vertex_list :: add_vertex(int num, int weight)
{
  Connected_vertexes *tmp;
  tmp=new Connected_vertexes[sizeof(Connected_vertexes)];
  tmp->num=num;
  tmp->weight=weight;
  tmp->next=vertexes;
  vertexes=tmp;
}


int Graph :: dijkstra(int v1, int v2)
{
  max_vert_and_weight_count();
  Vertex_list *tmp=list;
  //Connected_vertexes *tmp_vert;
  set_length(v1);
  step(v1, v2);
  while (tmp!=0)
  {
    if (tmp->num==v2)
      return tmp->shortest_path;
    tmp=tmp->next;
  }
  return 0;
}

void Graph :: step(int v1, int v2)
{
  Vertex_list *tmp=list;
  Connected_vertexes *tmp_vert;
  if (check_for_visiting(v1) && v1!=v2)
  {
    while (tmp->num!=v1)
      tmp=tmp->next;
    tmp_vert=tmp->vertexes;
    tmp->visited=1;
    while (tmp_vert!=0)
    {
      if (check_length(tmp_vert, tmp))
        step(tmp_vert->num, v2);
      tmp_vert=tmp_vert->next;
    }
    tmp_vert=tmp->vertexes;
    while (tmp_vert!=0)
    {
      step(tmp_vert->num, v2);
      tmp_vert=tmp_vert->next;
    }
    tmp=tmp->next;
  }
}


int Graph :: check_length(Connected_vertexes *vertex,
                           Vertex_list *ptr)
{
  Vertex_list *tmp=list;
  while (tmp!=0)
  {
    if (tmp->num==vertex->num)
    {
      if (tmp->shortest_path>vertex->weight+ptr->shortest_path)
      {
        tmp->visited=0;
        tmp->shortest_path=vertex->weight+ptr->shortest_path;
       return 1;
      }
    }
    tmp=tmp->next;
  }
  return 0;
}
 

int Graph :: check_for_visiting(int num)
{
  Vertex_list *tmp=list;
  while (tmp!=0)
  {
    if (tmp->num==num)
    {
      if (tmp->visited==0)
        return 1;
      else 
        return 0;
    }
    tmp=tmp->next;
  }
  return 0;
} 


void Graph :: set_length(int num)
{
  Vertex_list *tmp=list;
  while (tmp!=0)
  {
    tmp->visited=0;
    if (tmp->num==num)
      tmp->shortest_path=0;
    else
      tmp->shortest_path=max_weight;
    tmp=tmp->next;
  }
}


void Graph :: reweight()
{
  Vertex_list *tmp=list;
  Connected_vertexes *tmp_vert;
  while (tmp!=0)
  {
    tmp_vert=tmp->vertexes;
    while (tmp_vert!=0)
    { 
      tmp_vert->weight=h(tmp->num)-h(tmp_vert->num)+tmp_vert->weight;
      tmp_vert=tmp_vert->next;
    }
    tmp=tmp->next;
  }
}


int Graph :: h(int num)
{
  Vertex_list *tmp=list;
  while(tmp!=0)
  {
    if (tmp->num==num)
      return tmp->h;
    tmp=tmp->next;
  }
  return 0;
} 


int Graph :: virt(int num)
{
  Vertex_list *tmp=list;
  while(tmp!=0)
  {
    if (tmp->num==num)
      return tmp->virt_num;
    tmp=tmp->next;
  }
  return 0;
} 

void Graph :: bellman_f()
{
  int i=0, j=0, k=0, m=0, shrt;
  max_vert_and_weight_count();
  edges_and_verts_count();
  A=new int*[(vertex_num)*sizeof(int*)];
  for (i=0; i<vertex_num; i++)
    A[i]=new int[(edges_num)*sizeof(int)];
  for (j=0; j<vertex_num; j++)
    for (k=0; k<edges_num; k++)
    {
      A[j][k]=max_weight;
    }
  j=k=0;
  Vertex_list *tmp=list;
  Connected_vertexes *tmp_vert;
  for (m=0; m<vertex_num; m++)
    A[m][0]=0;
  for (k=1; k<=vertex_num-1; k++)
  {
    tmp=list;
    while (tmp!=0)
    {
      tmp_vert=tmp->vertexes;
      while (tmp_vert!=0)
      { 
        if (A[virt(tmp_vert->num)][k]>A[tmp->virt_num][k-1]+
            tmp_vert->weight)
          A[virt(tmp_vert->num)][k]=A[tmp->virt_num][k-1]+
          tmp_vert->weight;
        tmp_vert=tmp_vert->next;
      }
      tmp=tmp->next;
    }
  }
  tmp=list;
 /* for (j=0; j<vertex_num; j++)
  {
    for (k=0; k<edges_num; k++)
    {
     printf("%d ", A[j][k]);
    }
    printf("\n");
  }*/
  m=0;
  while (tmp!=0)
  {
    shrt=A[tmp->virt_num][1];
    for (k=0; k<vertex_num; k++)
      if (A[tmp->virt_num][k]<shrt)
        shrt=A[tmp->virt_num][k];
    tmp->h=shrt;
    m++;
    tmp=tmp->next;
  }
}


void Graph :: max_vert_and_weight_count()
{
  Vertex_list *tmp=list;
  Connected_vertexes *tmp_vert;
  max_vert=0;
  max_weight=0;
  edges_num=0;
  vertex_num=0;
  while (tmp!=0)
  {
    if (tmp->num>max_vert)
      max_vert=tmp->num;
    tmp_vert=tmp->vertexes;
    while (tmp_vert!=0)
    {
      if (tmp_vert->weight>max_weight)
        max_weight=tmp_vert->weight;
      tmp_vert=tmp_vert->next;
    }
    tmp=tmp->next;
  }
  max_vert++;
  max_weight++;
}


void Graph :: edges_and_verts_count()
{
  Vertex_list *tmp=list;
  Connected_vertexes *tmp_vert;
  edges_num=0;
  vertex_num=0;
  while (tmp!=0)
  {
    tmp_vert=tmp->vertexes;
    vertex_num++;
    while (tmp_vert!=0)
    {
      tmp_vert=tmp_vert->next;
      edges_num++;
    }
    tmp=tmp->next;
  }
}


void Graph :: add_special_vertex()
{
  Vertex_list *tmp=new Vertex_list[sizeof(Vertex_list)];
  Vertex_list *tmp_list=list;
  max_vert_and_weight_count();
  tmp->num=max_vert;
  tmp->vertexes=0;
  while (tmp_list!=0)
  {
    tmp->add_vertex(tmp_list->num, 0);
    tmp_list=tmp_list->next;
  }
  tmp->next=list;
  list=tmp;
}  


void Graph :: print_graph()
{ 
  Vertex_list *tmp=list;
  printf("Reweightned graph is\n");
  while (list!=0)
  {
    printf("start=%d, h=%d connected to:\n", list->num, list->h);
    Connected_vertexes *tmp_vert=list->vertexes;
    while (list->vertexes!=0)
    {
      printf("  vertex=%d weight=%d\n", list->vertexes->num,
                                        list->vertexes->weight);
      list->vertexes=list->vertexes->next;
    }
    list->vertexes=tmp_vert;
    list=list->next;
  }
  list=tmp;
}


Vertex_list* Graph :: search_vertex(int num) const
{
  Vertex_list *tmp=list;
  while (tmp!=0)
  {
    if (tmp->num==num)
      return list;
    tmp=tmp->next;
  }
  return 0;
}


void Graph :: set_graph(Edge_list *edges)
{
  list=0;
  Vertex_list *tmp;
  int i=0;
  while (edges!=0)
  {
    if ((tmp=search_vertex(edges->start_vertex))!=0)
      tmp->add_vertex(edges->finish_vertex, edges->weight);
    else
    {
      Vertex_list *tmp=new Vertex_list[sizeof(Vertex_list)];
      tmp->num=edges->start_vertex;
      tmp->vertexes=0;
      tmp->add_vertex(edges->finish_vertex, edges->weight);
      tmp->virt_num=i;
      tmp->next=list;
      list=tmp;
      i++;
    }
    Edge_list *tmp_edg=edges;
    edges=edges->next;
    delete[] tmp_edg;
  }
}
      

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

/*
~Buffer :: Buffer()
{
  delete[] content;
}*/


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


Server :: Server()
{
  flag=0;
  list=0;
}


void Server :: set_formation() 
{
  max_d=ls;
  FD_ZERO(&readfds);
  FD_SET(ls, &readfds);
  Client_list *tmp=list;
  while (list!=NULL)
  {
    FD_SET(list->socket_fd, &readfds);
    if (list->socket_fd>(max_d))
      max_d=list->socket_fd;
    list=list->next;
  }
  list=tmp;
}


void Server :: creation_of_listen_socket(char **argv)
{
  int opt=1;//,port=7777;
  struct sockaddr_in addr;
  addr.sin_family=AF_INET;
  addr.sin_port=htons(atoi(argv[1]));
  addr.sin_addr.s_addr=INADDR_ANY;
  ls=socket(AF_INET, SOCK_STREAM, 0);
  if (ls==-1)
  {
    perror("Can't create socket\n");
    exit(1);
  }
  setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  if (bind(ls, (struct sockaddr *) &addr, sizeof(addr))!=0)
  {
    perror("Error with IP attachment\n");
    exit(1);
  }
  if (listen(ls, 5)!=0)
  {
    perror("Can't transfer socket into listening state\n");
    exit(1);
  }
}


void Server :: add_socket_fd(int socket)
{
  struct Client_list *tmp;
  tmp=new Client_list[sizeof(*tmp)];
  tmp->socket_fd=socket;
  tmp->next=list;
  list=tmp;
}


void Server :: acception_of_client(int ls)
{
  int client_socket;
  //const char massage[]="\nWellcome to server\n";
  if ((client_socket=accept(ls, NULL, NULL))==-1)
  {
    perror("Can't connect to client\n");
    exit(1);
  }
  //write(client_socket, massage, sizeof(massage));
  add_socket_fd(client_socket);
}


void Server :: delete_socket(int socket)
{
  struct Client_list *prev=0, *curr=list;
  while (curr!=0)
  {
    curr=curr->next;
  }
  curr=list;
  while (curr!=0)
  {
    if (curr->socket_fd==socket)
    {
      if (prev==0)
      {
        list=curr->next;
        delete[] curr;
      }
      else
      {
        prev->next=curr->next;
        delete[] curr;
      }
      curr=0;
    }
    else
    {
      prev=curr;
      curr=curr->next;
    }
  }
}

/*
int Client_list :: client_count() const
{
  int n=0;
  while (list!=NULL)
  {
    n++;
    list=list->next;
  }
  return n;
}*/


void Server :: add_edge(char *string)
{
  Edge_list *tmp=new Edge_list[sizeof(Edge_list)];
  tmp->start_vertex=atoi(word_selection(string, 1));
  //printf("start=%d\n",tmp->start_vertex);
  tmp->finish_vertex=atoi(word_selection(string, 2));
  //printf("finish=%d\n",tmp->finish_vertex);
  tmp->weight=atoi(word_selection(string, 3));
  //printf("weight=%d\n",tmp->weight);
  tmp->next=list->edges;
  list->edges=tmp;
} 


void print_l(Client_list *list)
{
  while (list!=0)
  {
    printf("%d\n", list->socket_fd);
    list=list->next;
  }
}


void Server :: receiving_data()
{
  char buf[bufsize];
  int rc, f=0;
  char *string;
  char end[]="end";
  Client_list *tmp=list;
  while (tmp!=0)
  {
    f=0;
    if (FD_ISSET(tmp->socket_fd, &readfds))
    {
      rc=read(tmp->socket_fd, buf, sizeof(buf));
      if (rc<=0)
      {
        shutdown(tmp->socket_fd, 2);
        close(tmp->socket_fd);
        //Client_list *ptr=tmp;
        int socket=tmp->socket_fd;
        tmp=tmp->next;
        delete_socket(socket);
        f=1;
      }
      else
      {
        buf[rc]=0;
        tmp->buf.add_string(buf);
        while ((string=tmp->buf.string_search())!=0)
        {
          if (strcmp(word_selection(string, 3), end)==0)
          {
            process_information(string, tmp);
          }
          else
            add_edge(string);
        }   
      }
    }
    if (f==0)
      tmp=tmp->next;
  }
}


void Server :: process_information(char *str, Client_list *tmp)
{
  char end[]="end";
  printf("ready to process\n");
  if (flag==0)
  {
    graph.set_graph(list->edges);
    list->edges=0;
    //graph.add_special_vertex();
    graph.bellman_f();
    graph.reweight();
    flag=1;
  }
  graph.print_graph();
  int v1=atoi(word_selection(str, 1)), v2=atoi(word_selection(str, 2));
  int result=graph.dijkstra(v1, v2);
  printf("Shortest path=%d\n", result);
  char *mes=new char[bufsize + strlen(end)];
  sprintf(mes, "%d\n%s", result, end);
  write(tmp->socket_fd, mes, strlen(mes));
  delete[] mes;
}


void Server :: wait_for_activity()
{  
  int res; 
  printf("wait\n");
  if ((res=select(max_d+1, &readfds, NULL, NULL, NULL))<1)
  {
    perror("Select error\n");
    exit(1);
  }
  if (FD_ISSET(ls, &readfds))
    acception_of_client(ls);
} 

  
int main(int argc,char **argv)
{
  Server server;
  server.creation_of_listen_socket(argv);
  for (;;)
  {
    server.set_formation();
    server.wait_for_activity();
    server.receiving_data();
  }
  return 0;
}  
# graph-count-server
